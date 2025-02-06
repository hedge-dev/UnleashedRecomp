#pragma once

struct BlackBar
{
    static inline bool g_inspirePillarbox;

    static inline ImVec2 g_loadingBlackBarMin;
    static inline ImVec2 g_loadingBlackBarMax;
    static inline uint8_t g_loadingBlackBarAlpha;

    static void Draw();
};
