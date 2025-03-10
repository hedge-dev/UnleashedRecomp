#pragma once

#include <mod/mod_loader.h>
#include "config.h"

#define USER_DIRECTORY "UnleashedRecomp"

extern std::filesystem::path g_executableRoot;

inline std::filesystem::path GetGamePath()
{
#if defined(__linux__)
    if (g_isRunningUnderFlatpak)
        return "/var/data";
    else
#endif
        return ".";
}

bool CheckPortable();
std::filesystem::path BuildUserPath();
const std::filesystem::path& GetUserPath();
extern const std::filesystem::path g_gameInstallPath;

inline std::filesystem::path GetSavePath(bool checkForMods)
{
    if (checkForMods && !ModLoader::s_saveFilePath.empty())
        return ModLoader::s_saveFilePath.parent_path();
    else
        return GetUserPath() / "save";
}

// Returned file name may not necessarily be
// equal to SYS-DATA as mods can assign anything.
inline std::filesystem::path GetSaveFilePath(bool checkForMods)
{
    if (checkForMods && !ModLoader::s_saveFilePath.empty())
        return ModLoader::s_saveFilePath;
    else
        return GetSavePath(false) / "SYS-DATA";
}
