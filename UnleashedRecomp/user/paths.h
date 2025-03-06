#pragma once

#include <mod/mod_loader.h>

#define USER_DIRECTORY "UnleashedRecomp"

#ifndef GAME_INSTALL_DIRECTORY_PREPROC
#define GAME_INSTALL_DIRECTORY_PREPROC "."
#endif

bool CheckPortable();
std::filesystem::path BuildUserPath();
const std::filesystem::path& GetUserPath();
extern const std::string g_gamepath;
std::filesystem::path GetGamePath();

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
