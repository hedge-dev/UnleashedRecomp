#pragma once

class MessageWindow
{
public:
    inline static bool s_isVisible = false;

    static void Init();
    static void Draw();
    static bool Open(std::string text, int* result, std::span<std::string> buttons = {}, int defaultButtonIndex = 0);
    static void Close();
};
