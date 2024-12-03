#pragma once

#include <api/SWA.h>

class OptionsMenu
{
public:
    inline static bool s_isVisible = false;
    inline static bool s_isPause = false;

    inline static SWA::EMenuType s_pauseMenuType;

    static void Init();
    static void Draw();
    static void Open(bool isPause = false, SWA::EMenuType pauseMenuType = SWA::eMenuType_WorldMap);
    static void Close();

    static bool CanClose();
};
