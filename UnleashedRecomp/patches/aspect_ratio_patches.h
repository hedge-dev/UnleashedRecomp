#pragma once

inline constexpr float NARROW_ASPECT_RATIO = 4.0f / 3.0f;
inline constexpr float WIDE_ASPECT_RATIO = 16.0f / 9.0f;
inline constexpr float STEAM_DECK_ASPECT_RATIO = 16.0f / 10.0f;

inline float g_aspectRatio;
inline float g_aspectRatioOffsetX;
inline float g_aspectRatioOffsetY;
inline float g_aspectRatioScale;
inline float g_narrowOffsetScale;

struct AspectRatioPatches
{
    static void ComputeOffsets();
};
