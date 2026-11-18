#pragma once

// Discord Rich Presence over the local discord-ipc socket
namespace DiscordPresence {
void Init();     // start the worker thread (call once, after Config::Load)
void Update();   // cheap + self-throttled; call every frame
void Shutdown(); // stop the worker (call on exit)
} // namespace DiscordPresence
