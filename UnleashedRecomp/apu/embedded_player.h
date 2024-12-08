#pragma once

struct EmbeddedPlayer 
{
    inline static bool s_isActive = false;

    static void Init();
    static void Play(const char *name);
    static void Shutdown();
};
