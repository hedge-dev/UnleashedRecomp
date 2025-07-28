#pragma once

#include <api/SWA.h>

struct InstallerWizard
{
    static inline bool s_isVisible = false;
    static inline bool s_returnToTitle = false;

    static void Init();
    static void Draw();
    static void Shutdown();
    static bool Run(std::filesystem::path installPath, bool skipGame);
};
