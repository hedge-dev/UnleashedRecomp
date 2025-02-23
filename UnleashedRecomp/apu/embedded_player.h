#pragma once

struct EmbeddedPlayer 
{
    // Arbitrarily picked volume to match the mixing in the original game.
    static constexpr float MUSIC_VOLUME = 0.25f;

    static inline bool s_isActive = false;

    static void Init();
    static void Play(const char *name);
    static void PlayMusic();
    static void FadeOutMusic();
    static void Shutdown();
};
