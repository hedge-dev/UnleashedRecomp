#pragma once

class AchievementMenu
{
public:
    inline static bool s_isVisible = false;

    static void Init();
    static void Draw();
    static void Open();
    static void Close();
};
