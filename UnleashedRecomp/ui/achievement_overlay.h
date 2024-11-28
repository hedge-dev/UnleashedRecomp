#pragma once

#include "imgui_view.h"

class AchievementOverlay : ImGuiView
{
public:
    inline static bool s_isVisible = false;

    void Init() override;
    void Draw() override;
    static void Open(int id);
    static void Close();
};
