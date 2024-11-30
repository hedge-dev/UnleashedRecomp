#pragma once

#include <queue>

class AchievementOverlay
{
public:
    inline static bool s_isVisible = false;

    inline static std::queue<uint16_t> s_queue{};

    static void Init();
    static void Draw();
    static void Open(int id);
    static void Close();
};
