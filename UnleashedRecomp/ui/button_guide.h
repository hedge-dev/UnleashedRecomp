#pragma once

#include <gpu/video.h>

enum class EButtonIcon
{
    // Controller
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

    // Keyboard + Mouse (temporary)
    LMB,
    Enter,
    Escape
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
    float MaxWidth{ FLT_MAX };
    EButtonIcon Icon{};
    EButtonAlignment Alignment{ EButtonAlignment::Right };
    EFontQuality FontQuality{ EFontQuality::High };
    bool* Visibility{ nullptr };

    Button(std::string name, float maxWidth, EButtonIcon icon, EButtonAlignment alignment, EFontQuality fontQuality = EFontQuality::High, bool* visibility = nullptr)
        : Name(name), MaxWidth(maxWidth), Icon(icon), Alignment(alignment), FontQuality(fontQuality), Visibility(visibility) {}

    Button(std::string name, float maxWidth, EButtonIcon icon, EButtonAlignment alignment, bool* visibility)
        : Name(name), MaxWidth(maxWidth), Icon(icon), Alignment(alignment), Visibility(visibility) {}

    Button(std::string name, float maxWidth, EButtonIcon icon, bool* visibility)
        : Name(name), MaxWidth(maxWidth), Icon(icon), Visibility(visibility) {}

    Button(std::string name, float maxWidth, EButtonIcon icon)
        : Name(name), MaxWidth(maxWidth), Icon(icon) {}
};

class ButtonGuide
{
public:
    static inline bool s_isVisible = false;

    static void Init();
    static void Draw();
    static void Open(Button button);
    static void Open(const std::span<Button> buttons);
    static void SetSideMargins(float width);
    static void Close();
};
