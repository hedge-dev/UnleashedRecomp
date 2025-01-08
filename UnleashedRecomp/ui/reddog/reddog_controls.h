#pragma once

namespace Reddog
{
    enum class EButtonTextAlignment
    {
        Left,
        Centre,
        Right
    };

    void InitControlsResources();
    bool Button(const char* label);
    bool Checkbox(const char* label, bool* v);
    bool ExplicitButton(const char* label, EButtonTextAlignment textAlignment = EButtonTextAlignment::Centre, const ImVec2& size = { 0, 0 }, float fontScale = 1);
    void Separator(float upperPadding = 0, float lowerPadding = 0);
}
