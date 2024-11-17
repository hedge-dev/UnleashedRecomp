#pragma once

struct OptionsMenu
{
public:
    inline static bool s_isVisible = false;
    inline static bool s_isDimBackground = false;

    static void Init();
    static void Draw();
    static void Open(bool stage = false);
    static void Close(bool stage = false);
};
