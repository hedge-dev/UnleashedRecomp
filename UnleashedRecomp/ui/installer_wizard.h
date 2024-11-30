#pragma once

#include <api/SWA.h>

struct InstallerWizard
{
    inline static bool s_isVisible = false;

    static void Init();
    static void Draw();
    static void Shutdown();
    static bool Run(bool skipGame);
};
