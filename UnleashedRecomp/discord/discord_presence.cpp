#include "discord_presence.h"
#include <stdafx.h>

#include <atomic>
#include <cctype>
#include <ctime>
#include <thread>

#include <nlohmann/json.hpp>

#include <api/SWA.h>
#include <app.h>
#include <os/logger.h>
#include <user/config.h>

#ifndef _WIN32
#include <cerrno>
#include <fcntl.h>
#include <poll.h>
#include <sys/socket.h>
#include <sys/un.h>
#endif

#define DISCORD_APP_ID "1547039038194716693"

using json = nlohmann::json;

namespace {
enum : uint32_t { OP_HANDSHAKE = 0, OP_FRAME = 1, OP_CLOSE = 2 };

struct Activity {
  std::string details;
  std::string state;
  bool operator==(const Activity &) const = default;
};

std::mutex g_mutex;
Activity g_pending;   // guarded by g_mutex
Activity g_lastBuilt; // game thread only
std::atomic<bool> g_running{false};
std::atomic<uint32_t> g_nonce{0};
std::thread g_worker;
int64_t g_startTime = 0;

#ifdef _WIN32
using socket_t = HANDLE;
#define INVALID_SOCK INVALID_HANDLE_VALUE
#else
using socket_t = int;
#define INVALID_SOCK (-1)
#ifndef MSG_NOSIGNAL
#define MSG_NOSIGNAL 0
#endif
#endif

// ---------------------------------------------------------------- transport

#ifdef _WIN32
socket_t IpcConnect() {
  for (int i = 0; i < 10; i++) {
    char path[64];
    snprintf(path, sizeof(path), "\\\\?\\pipe\\discord-ipc-%d", i);

    HANDLE h = CreateFileA(path, GENERIC_READ | GENERIC_WRITE, 0, nullptr,
                           OPEN_EXISTING, 0, nullptr);
    if (h != INVALID_HANDLE_VALUE)
      return h;
  }
  return INVALID_SOCK;
}

void IpcClose(socket_t s) {
  if (s != INVALID_SOCK)
    CloseHandle(s);
}

bool IpcWrite(socket_t s, const void *data, size_t len) {
  auto *p = static_cast<const char *>(data);
  while (len) {
    DWORD wrote = 0;
    if (!WriteFile(s, p, static_cast<DWORD>(len), &wrote, nullptr) ||
        wrote == 0)
      return false;
    p += wrote;
    len -= wrote;
  }
  return true;
}

// >0 bytes read, 0 if nothing pending, <0 if the pipe died.
int IpcRead(socket_t s, void *buf, size_t len) {
  DWORD avail = 0;
  if (!PeekNamedPipe(s, nullptr, 0, nullptr, &avail, nullptr))
    return -1;
  if (avail == 0)
    return 0;

  DWORD got = 0;
  if (!ReadFile(s, buf, static_cast<DWORD>(std::min<size_t>(len, avail)), &got,
                nullptr))
    return -1;
  return static_cast<int>(got);
}
#else
socket_t IpcConnect() {
  const char *bases[] = {getenv("XDG_RUNTIME_DIR"), getenv("TMPDIR"),
                         getenv("TMP"), getenv("TEMP"), "/tmp"};
  // "" = plain path; the others cover Flatpak/Snap Discord.
  const char *subs[] = {"", "app/com.discordapp.Discord/", "snap.discord/"};

  for (const char *base : bases) {
    if (!base || !*base)
      continue;

    for (const char *sub : subs) {
      for (int i = 0; i < 10; i++) {
        sockaddr_un addr{};
        addr.sun_family = AF_UNIX;
        int n = snprintf(addr.sun_path, sizeof(addr.sun_path),
                         "%s/%sdiscord-ipc-%d", base, sub, i);
        if (n < 0 || n >= static_cast<int>(sizeof(addr.sun_path)))
          continue;

        int fd = socket(AF_UNIX, SOCK_STREAM, 0);
        if (fd < 0)
          return INVALID_SOCK;

#ifdef __APPLE__
        int on = 1;
        setsockopt(fd, SOL_SOCKET, SO_NOSIGPIPE, &on, sizeof(on));
#endif
        if (connect(fd, reinterpret_cast<sockaddr *>(&addr), sizeof(addr)) ==
            0) {
          fcntl(fd, F_SETFL, O_NONBLOCK);
          return fd;
        }
        close(fd);
      }
    }
  }
  return INVALID_SOCK;
}

void IpcClose(socket_t s) {
  if (s != INVALID_SOCK)
    close(s);
}

bool IpcWrite(socket_t s, const void *data, size_t len) {
  auto *p = static_cast<const char *>(data);
  while (len) {
    ssize_t w = send(s, p, len, MSG_NOSIGNAL);
    if (w > 0) {
      p += w;
      len -= static_cast<size_t>(w);
      continue;
    }
    if (w < 0 && (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)) {
      pollfd pfd{s, POLLOUT, 0};
      poll(&pfd, 1, 200);
      continue;
    }
    return false;
  }
  return true;
}

int IpcRead(socket_t s, void *buf, size_t len) {
  ssize_t r = recv(s, buf, len, 0);
  if (r > 0)
    return static_cast<int>(r);
  if (r == 0)
    return -1; // peer closed
  if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)
    return 0;
  return -1;
}
#endif

bool SendFrame(socket_t s, uint32_t op, const std::string &payload) {
  uint32_t header[2] = {op, static_cast<uint32_t>(payload.size())};
  return IpcWrite(s, header, sizeof(header)) &&
         (payload.empty() || IpcWrite(s, payload.data(), payload.size()));
}

bool Handshake(socket_t s) {
  return SendFrame(s, OP_HANDSHAKE,
                   json({{"v", 1}, {"client_id", DISCORD_APP_ID}}).dump());
}

bool SendActivity(socket_t s, const Activity &a) {
  json activity = json::object();
  if (!a.details.empty())
    activity["details"] = a.details;
  if (!a.state.empty())
    activity["state"] = a.state;
  activity["timestamps"] = {{"start", g_startTime}};

#ifdef _WIN32
  int pid = static_cast<int>(GetCurrentProcessId());
#else
  int pid = static_cast<int>(getpid());
#endif
  json frame = {
      {"cmd", "SET_ACTIVITY"},
      {"nonce", std::to_string(g_nonce.fetch_add(1))},
      {"args", {{"pid", pid}, {"activity", activity}}},
  };
  return SendFrame(s, OP_FRAME, frame.dump());
}

// ------------------------------------------------------------- game -> text

// "Act_EggmanLand" -> "Eggman Land", "Act_Apotos_2" -> "Apotos 2".
std::string PrettifyStageId(const char *id) {
  std::string s = id ? id : "";
  for (const char *prefix : {"Act_", "Town_", "Boss_", "Mykonos_"}) {
    if (s.rfind(prefix, 0) == 0) {
      s = s.substr(std::string_view(prefix).size());
      break;
    }
  }

  std::string out;
  for (size_t i = 0; i < s.size(); i++) {
    char c = s[i];
    if (c == '_') {
      out += ' ';
      continue;
    }
    if (i && std::isupper(static_cast<unsigned char>(c)) &&
        !std::isupper(static_cast<unsigned char>(s[i - 1]))) {
      out += ' ';
    }
    out += c;
  }
  return out.empty() ? "In game" : out;
}

// Unleashed uses codenamed stage ids
const char *StageRegion(std::string_view id) {
  static const std::pair<std::string_view, const char *> kRegions[] = {
      {"Mykonos", "Apotos"},        {"China", "Chun-nan"},
      {"EULabo", "Spagonia"},       {"EuropeanCity", "Spagonia"},
      {"EU", "Spagonia"},           {"Africa", "Mazuri"},
      {"Snow", "Holoska"},          {"NYCity", "Empire City"},
      {"NY", "Empire City"},        {"SouthEastAsia", "Adabat"},
      {"Beach", "Adabat"},          {"PetraCapital", "Shamar"},
      {"PetraLabo", "Shamar"},      {"Petra", "Shamar"},
      {"EggmanLand", "Eggmanland"},
  };
  for (auto &[token, region] : kRegions)
    if (id.find(token) != std::string_view::npos)
      return region;
  return nullptr;
}

// Internal stage id -> display name
std::string StageDisplayName(const char *id) {
  if (!id || !*id)
    return "In game";

  std::string_view s = id;

  static const std::unordered_map<std::string_view, std::string_view> kStages =
      {
          {"ActD_MykonosAct1", "Apotos - Windmill Isle Act 1"},
          {"ActD_MykonosAct2", "Apotos - Windmill Isle Act 2"},
          {"ActD_China", "Chun-nan - Dragon Road"},
          {"ActD_EU", "Spagonia - Rooftop Run"},
          {"ActD_Africa", "Mazuri - Savannah Citadel"},
          {"ActD_Snow", "Holoska - Cool Edge"},
          {"ActD_NY", "Empire City - Skyscraper Scamper"},
          {"ActD_Beach", "Adabat - Jungle Joyride"},
          {"ActD_Petra", "Shamar - Arid Sands"},
          {"ActN_MykonosEvil", "Apotos - Windmill Isle (Night)"},
          {"ActN_ChinaEvil", "Chun-nan - Dragon Road (Night)"},
          {"ActN_EUEvil", "Spagonia - Rooftop Run (Night)"},
          {"ActN_AfricaEvil", "Mazuri - Savannah Citadel (Night)"},
          {"ActN_SnowEvil", "Holoska - Cool Edge (Night)"},
          {"ActN_NYEvil", "Empire City - Skyscraper Scamper (Night)"},
          {"ActN_BeachEvil", "Adabat - Jungle Joyride (Night)"},
          {"ActN_PetraEvil", "Shamar - Arid Sands (Night)"},
          {"Act_EggmanLand", "Eggmanland"},
          {"BossEggBeetle", "Boss: Egg Beetle"},
          {"BossEggLancer", "Boss: Egg Lancer"},
          {"BossEggRayBird", "Boss: Egg Devil Ray"},
          {"Title", "Main Menu"},
          {"StaffRoll", "Credits"},
      };
  if (auto it = kStages.find(s); it != kStages.end())
    return std::string(it->second);

  if (s.rfind("Event_", 0) == 0 || s == "Inspire")
    return "Watching a cutscene";

  if (const char *region = StageRegion(s)) {
    bool night = s.rfind("ActN_", 0) == 0 ||
                 s.find("Evil") != std::string_view::npos ||
                 s.find("_Night") != std::string_view::npos;
    std::string out = region;
    if (s.rfind("Town_", 0) == 0 || s.rfind("CmnTown_", 0) == 0)
      out += night ? " - Hub (Night)" : " - Hub";
    else if (s.find("Sub") != std::string_view::npos)
      out += night ? " - Night mission" : " - Extra mission";
    else if (night)
      out += " (Night)";
    else if (s.rfind("ActD_", 0) == 0)
      out += " (Day)";
    return out;
  }

  return PrettifyStageId(id);
}

Activity BuildActivity() {
  Activity a;

  if (!App::s_isInit) {
    a.details = "Starting up";
    return a;
  }

  if (App::s_isLoading) {
    a.details = "Loading\xE2\x80\xA6"; // "Loading…"
    return a;
  }

  const char *stage = nullptr;
  if (auto *doc = SWA::CGameDocument::GetInstance(); doc && doc->m_pMember) {
    const char *s = doc->m_pMember->m_StageName.c_str();
    if (s && *s)
      stage = s;
  }

  if (stage) {
    a.details = StageDisplayName(stage);
    a.state = App::s_isWerehog ? "Playing as the Werehog" : "Playing as Sonic";
  } else {
    a.details = "In the menus";
  }

  return a;
}

// -------------------------------------------------------------- worker loop

void WorkerMain() {
  socket_t sock = INVALID_SOCK;
  Activity sent;
  bool haveSent = false;
  int backoffMs = 1000;
  auto lastSend = std::chrono::steady_clock::now() - std::chrono::hours(1);

  while (g_running.load(std::memory_order_relaxed)) {
    if (!Config::DiscordRichPresence) {
      if (sock != INVALID_SOCK) {
        IpcClose(sock);
        sock = INVALID_SOCK;
        haveSent = false;
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(500));
      continue;
    }

    if (sock == INVALID_SOCK) {
      sock = IpcConnect();
      if (sock == INVALID_SOCK || !Handshake(sock)) {
        IpcClose(sock);
        sock = INVALID_SOCK;
        std::this_thread::sleep_for(std::chrono::milliseconds(backoffMs));
        backoffMs = std::min(backoffMs * 2, 15000);
        continue;
      }
      LOGN("Discord: connected");
      backoffMs = 1000;
      haveSent = false;
    }

    char scratch[2048];
    if (IpcRead(sock, scratch, sizeof(scratch)) < 0) {
      IpcClose(sock);
      sock = INVALID_SOCK;
      haveSent = false;
      continue;
    }

    Activity want;
    {
      std::lock_guard lk(g_mutex);
      want = g_pending;
    }

    auto now = std::chrono::steady_clock::now();
    bool changed = !haveSent || !(want == sent);
    if (changed && now - lastSend >= std::chrono::seconds(4)) {
      if (!SendActivity(sock, want)) {
        IpcClose(sock);
        sock = INVALID_SOCK;
        haveSent = false;
        continue;
      }
      sent = want;
      haveSent = true;
      lastSend = now;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(500));
  }

  if (sock != INVALID_SOCK) {
    SendFrame(sock, OP_CLOSE, "{}");
    IpcClose(sock);
  }
}
} // namespace

namespace DiscordPresence {
void Init() {
  if (std::string_view(DISCORD_APP_ID) == "0000000000000000") {
    LOGN_WARNING("Discord: no application id set, rich presence disabled");
    return;
  }
  if (g_running.exchange(true))
    return;

  g_startTime = static_cast<int64_t>(std::time(nullptr));
  g_worker = std::thread(WorkerMain);
}

void Update() {
  if (!g_running.load(std::memory_order_relaxed))
    return;

  // Rebuild at most once a second, off the game clock.
  static double nextBuild = 0.0;
  if (App::s_time < nextBuild)
    return;
  nextBuild = App::s_time + 1.0;

  Activity a = BuildActivity();
  if (a == g_lastBuilt)
    return;
  g_lastBuilt = a;

  std::lock_guard lk(g_mutex);
  g_pending = std::move(a);
}

void Shutdown() {
  if (!g_running.exchange(false))
    return;
  if (g_worker.joinable())
    g_worker.join();
}
} // namespace DiscordPresence
