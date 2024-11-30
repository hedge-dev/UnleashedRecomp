#pragma once

class AchievementOverlay
{
public:
    inline static bool s_isVisible = false;

    static void Init();
    static void Draw();
    static void Open(int id);
    static void Close();
};
