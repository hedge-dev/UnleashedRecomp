#pragma once

#include <mod/mod_loader.h>

#define USER_DIRECTORY "UnleashedRecomp"

#ifndef GAME_INSTALL_DIRECTORY
#define GAME_INSTALL_DIRECTORY "."
#endif

extern std::filesystem::path g_executableRoot;

inline std::string GetGamePath()
{
    const char* homeDir = getenv("HOME");
    if (homeDir == nullptr)
        return g_executableRoot.string();
    std::filesystem::path homePath = homeDir;
    std::filesystem::path gamePath = homePath / ".local/share" / USER_DIRECTORY;
    return gamePath.string();
}

bool CheckPortable();
std::filesystem::path BuildUserPath();
const std::filesystem::path& GetUserPath();
extern const std::string g_gamepath;

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
