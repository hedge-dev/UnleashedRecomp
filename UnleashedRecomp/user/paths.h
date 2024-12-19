#pragma once

#define USER_DIRECTORY "SWA"

#ifndef GAME_INSTALL_DIRECTORY
#define GAME_INSTALL_DIRECTORY "."
#endif

inline std::filesystem::path GetGamePath()
{
    return GAME_INSTALL_DIRECTORY;
}

inline std::filesystem::path GetUserPath()
{
    if (std::filesystem::exists(GAME_INSTALL_DIRECTORY "/portable.txt"))
        return GAME_INSTALL_DIRECTORY;

    std::filesystem::path userPath;

#if defined(_WIN32)
    PWSTR knownPath = NULL;
    if (SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, NULL, &knownPath) == S_OK)
        userPath = std::filesystem::path{ knownPath } / USER_DIRECTORY;

    CoTaskMemFree(knownPath);
#elif defined(__linux__)
    const char *homeDir = getenv("HOME");
    if (homeDir == nullptr)
    {
        homeDir = getpwuid(getuid())->pw_dir;
    }

    if (homeDir != nullptr)
    {
        // Prefer to store in the .config directory if it exists. Use the home directory otherwise.
        std::filesystem::path homePath = homeDir;
        std::filesystem::path configPath = homePath / ".config";
        if (std::filesystem::exists(configPath))
            userPath = configPath / USER_DIRECTORY;
        else
            userPath = homePath / ("." USER_DIRECTORY);
    }
#else
    static_assert(false, "GetUserPath() not implemented for this platform.");
#endif

    return userPath;
}

inline std::filesystem::path GetSavePath()
{
    return GetUserPath() / "save";
}
