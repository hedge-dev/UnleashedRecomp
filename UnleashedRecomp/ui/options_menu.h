#pragma once

#include "imgui_view.h"
#include <api/SWA.h>

struct OptionsMenu : ImGuiView
{
public:
    inline static bool s_isVisible = false;
    inline static bool s_isPause = false;

    inline static SWA::EMenuType s_pauseMenuType;

    void Init() override;
    void Draw() override;
    static void Open(bool isPause = false, SWA::EMenuType pauseMenuType = SWA::eMenuType_WorldMap);
    static void Close();

    static bool CanClose();
};
