#include "discord_presence.h"
#include <stdafx.h>

#include <atomic>
#include <cctype>
#include <ctime>
#include <thread>

#include <nlohmann/json.hpp>

#include <api/SWA.h>
#include <app.h>
#include <locale/locale.h>
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
  std::string largeImage, largeText; // art-asset key (or https URL) + tooltip
  std::string smallImage, smallText;
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

// Discord IPC transport: a named pipe on Windows, a unix socket elsewhere.

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

  json assets = json::object();
  if (!a.largeImage.empty()) {
    assets["large_image"] = a.largeImage;
    if (!a.largeText.empty())
      assets["large_text"] = a.largeText;
  }
  if (!a.smallImage.empty()) {
    assets["small_image"] = a.smallImage;
    if (!a.smallText.empty())
      assets["small_text"] = a.smallText;
  }
  if (!assets.empty())
    activity["assets"] = std::move(assets);

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

// Player-facing strings go through Localise(); the "RichPresence_*" keys live
// in locale/locale.cpp. Only stage-id -> key resolution happens here.

// Fallback for unrecognised stage ids (e.g. custom stages from mods):
// "ActD_SomeMod" -> "Some Mod".
std::string PrettifyStageId(const char *id) {
  std::string s = id ? id : "";
  for (const char *prefix :
       {"ActD_", "ActN_", "Act_", "CmnTown_", "Town_", "Boss"}) {
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
  return out.empty() ? Localise("RichPresence_Status_InGame") : out;
}

// Unleashed uses codenamed stage ids: Apotos = "Mykonos", Chun-nan = "China",
// Spagonia = "EU"/"EuropeanCity", Mazuri = "Africa", Holoska = "Snow", Empire
// City = "NY", Adabat = "Beach"/"SouthEastAsia", Shamar = "Petra". Day acts are
// "ActD_*", Werehog night acts "ActN_*Evil".

struct Region {
  std::string_view token; // codename substring found in the stage id
  const char *nameKey;    // locale key for the region name
  const char *image;      // Discord art-asset key (upload in the Dev Portal)
};

// Match a stage id to its region.
const Region *FindRegion(std::string_view id) {
  static const Region kRegions[] = {
      {"Mykonos", "RichPresence_Region_Apotos", "apotos"},
      {"China", "RichPresence_Region_Chunnan", "chunnan"},
      {"EULabo", "RichPresence_Region_Spagonia", "spagonia"},
      {"EuropeanCity", "RichPresence_Region_Spagonia", "spagonia"},
      {"EU", "RichPresence_Region_Spagonia", "spagonia"},
      {"Africa", "RichPresence_Region_Mazuri", "mazuri"},
      {"Snow", "RichPresence_Region_Holoska", "holoska"},
      {"NYCity", "RichPresence_Region_EmpireCity", "empirecity"},
      {"NY", "RichPresence_Region_EmpireCity", "empirecity"},
      {"SouthEastAsia", "RichPresence_Region_Adabat", "adabat"},
      {"Beach", "RichPresence_Region_Adabat", "adabat"},
      {"PetraCapital", "RichPresence_Region_Shamar", "shamar"},
      {"PetraLabo", "RichPresence_Region_Shamar", "shamar"},
      {"Petra", "RichPresence_Region_Shamar", "shamar"},
      {"EggmanLand", "RichPresence_Region_Eggmanland", "eggmanland"},
  };
  for (const auto &r : kRegions)
    if (id.find(r.token) != std::string_view::npos)
      return &r;
  return nullptr;
}

// Stage id -> Discord art-asset key
std::string StageImageKey(const char *id) {
  if (!id || !*id)
    return "";
  std::string_view s = id;
  const Region *r = FindRegion(s);
  if (!r)
    return "";
  bool night = s.rfind("ActN_", 0) == 0 ||
               s.find("Evil") != std::string_view::npos ||
               s.find("_Night") != std::string_view::npos;
  return night ? std::string(r->image) + "_night" : r->image;
}

struct StoryStage {
  const char *key; // locale key for the day-time stage name
  bool night;      // append " (Night)" for the Werehog version
};

// Internal stage id -> localised display name.
std::string StageDisplayName(const char *id) {
  if (!id || !*id)
    return Localise("RichPresence_Status_InGame");

  std::string_view s = id;

  static const std::unordered_map<std::string_view, StoryStage> kStages = {
      {"ActD_MykonosAct1", {"RichPresence_Stage_Apotos_Act1", false}},
      {"ActD_MykonosAct2", {"RichPresence_Stage_Apotos_Act2", false}},
      {"ActD_China", {"RichPresence_Stage_Chunnan", false}},
      {"ActD_EU", {"RichPresence_Stage_Spagonia", false}},
      {"ActD_Africa", {"RichPresence_Stage_Mazuri", false}},
      {"ActD_Snow", {"RichPresence_Stage_Holoska", false}},
      {"ActD_NY", {"RichPresence_Stage_EmpireCity", false}},
      {"ActD_Beach", {"RichPresence_Stage_Adabat", false}},
      {"ActD_Petra", {"RichPresence_Stage_Shamar", false}},
      {"ActN_MykonosEvil", {"RichPresence_Stage_Apotos", true}},
      {"ActN_ChinaEvil", {"RichPresence_Stage_Chunnan", true}},
      {"ActN_EUEvil", {"RichPresence_Stage_Spagonia", true}},
      {"ActN_AfricaEvil", {"RichPresence_Stage_Mazuri", true}},
      {"ActN_SnowEvil", {"RichPresence_Stage_Holoska", true}},
      {"ActN_NYEvil", {"RichPresence_Stage_EmpireCity", true}},
      {"ActN_BeachEvil", {"RichPresence_Stage_Adabat", true}},
      {"ActN_PetraEvil", {"RichPresence_Stage_Shamar", true}},
  };
  if (auto it = kStages.find(s); it != kStages.end()) {
    std::string out = Localise(it->second.key);
    if (it->second.night)
      out += " " + Localise("RichPresence_Suffix_Night");
    return out;
  }

  static const std::unordered_map<std::string_view, const char *> kNamed = {
      {"Act_EggmanLand", "RichPresence_Stage_Eggmanland"},
      {"BossEggBeetle", "RichPresence_Boss_EggBeetle"},
      {"BossEggLancer", "RichPresence_Boss_EggLancer"},
      {"BossEggRayBird", "RichPresence_Boss_EggDevilRay"},
      {"Title", "RichPresence_Status_Menus"},
      {"StaffRoll", "RichPresence_Credits"},
  };
  if (auto it = kNamed.find(s); it != kNamed.end())
    return Localise(it->second);

  if (s.rfind("Event_", 0) == 0 || s == "Inspire")
    return Localise("RichPresence_Status_Cutscene");

  // Derived name for hubs, extra missions and DLC/ETF variants.
  if (const Region *region = FindRegion(s)) {
    bool night = s.rfind("ActN_", 0) == 0 ||
                 s.find("Evil") != std::string_view::npos ||
                 s.find("_Night") != std::string_view::npos;
    std::string out = Localise(region->nameKey);
    if (s.rfind("Town_", 0) == 0 || s.rfind("CmnTown_", 0) == 0)
      out += " - " + Localise(night ? "RichPresence_Suffix_HubNight"
                                    : "RichPresence_Suffix_Hub");
    else if (s.find("Sub") != std::string_view::npos)
      out += " - " + Localise(night ? "RichPresence_Suffix_NightMission"
                                    : "RichPresence_Suffix_ExtraMission");
    else if (night)
      out += " " + Localise("RichPresence_Suffix_Night");
    else if (s.rfind("ActD_", 0) == 0)
      out += " " + Localise("RichPresence_Suffix_Day");
    return out;
  }

  LOGFN_WARNING("Discord: unmapped stage id \"{}\"", id);
  return PrettifyStageId(id);
}

Activity BuildActivity() {
  Activity a;

  if (!App::s_isInit) {
    a.details = Localise("RichPresence_Status_Startup");
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
    a.state = Localise(App::s_isWerehog ? "RichPresence_State_Werehog"
                                        : "RichPresence_State_Sonic");
    a.largeImage = StageImageKey(stage);
    a.largeText = a.details;
    a.smallImage = App::s_isWerehog ? "werehog" : "sonic";
    a.smallText = a.state;
  } else {
    a.details = Localise("RichPresence_Status_WorldMap");
    // Set to an uploaded key (e.g. "worldmap") if you want art here; empty
    // falls back to the application icon.
    a.largeImage = "";
    a.largeText = a.details;
  }

  return a;
}

// Owns the socket: connects, reconnects with backoff, and pushes the latest
// activity to Discord (rate-limited).
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
