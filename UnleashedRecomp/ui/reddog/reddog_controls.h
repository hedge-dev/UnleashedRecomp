#pragma once

namespace Reddog
{
    void InitControlsResources();
    bool Button(const char* label);
    bool Checkbox(const char* label, bool* v);
    void Separator(float upperPadding = 0, float lowerPadding = 0);
}
