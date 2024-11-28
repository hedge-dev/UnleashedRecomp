#pragma once

#include "imgui_view.h"

class AchievementMenu : ImGuiView
{
public:
    inline static bool s_isVisible = false;

    void Init() override;
    void Draw() override;
    static void Open();
    static void Close();
};
