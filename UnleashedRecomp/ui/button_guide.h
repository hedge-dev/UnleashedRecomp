#pragma once

#include <gpu/video.h>

enum class EButtonIcon
{
    A,
    B,
    X,
    Y,
    LB,
    RB,
    LBRB,
    LT,
    RT,
    LTRT,
    Start,
    Back,
    LMB
};

enum class EButtonAlignment
{
    Left,
    Right
};

enum class EFontQuality
{
    Low,
    High
};

class Button
{
public:
    std::string Name{};
    EButtonIcon Icon{};
    EButtonAlignment Alignment{ EButtonAlignment::Right };
    EFontQuality FontQuality{ EFontQuality::High };
    bool* Visibility{ nullptr };

    Button(std::string name, EButtonIcon icon, EButtonAlignment alignment, EFontQuality fontQuality = EFontQuality::High, bool* visibility = nullptr)
        : Name(name), Icon(icon), Alignment(alignment), FontQuality(fontQuality), Visibility(visibility) {}

    Button(std::string name, EButtonIcon icon, bool* visibility) : Name(name), Icon(icon), Visibility(visibility) {}

    Button(std::string name, EButtonIcon icon) : Name(name), Icon(icon) {}
};

class ButtonGuide
{
public:
    inline static bool s_isVisible = false;

    static void Init();
    static void Draw();
    static void Open(const std::vector<Button> buttons);
    static void SetSideMargins(float width);
    static void Close();
};
