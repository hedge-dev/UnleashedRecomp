#include "paths.h"
#include <user/config.h>
#include <os/process.h>

std::filesystem::path g_executableRoot = os::process::GetExecutablePath().remove_filename();
std::filesystem::path g_userPath = BuildUserPath();
extern const std::string g_gamepath = GetGamePath().string();

bool CheckPortable()
{
    return std::filesystem::exists(g_executableRoot / "portable.txt");
}

std::filesystem::path GetGamePath()
{
#if defined(__linux__)
    if (g_isRuntimeFlatpak || CheckPortable())
        return GAME_INSTALL_DIRECTORY_PREPROC;

    const char* homeDir = getenv("HOME");
    if (homeDir == nullptr)
    {
        return GAME_INSTALL_DIRECTORY_PREPROC;
    }
    else 
    {
        std::filesystem::path homePath = homeDir;
        std::filesystem::path gamePath = homePath / ".local/share" / USER_DIRECTORY;
        return gamePath;
    }
#else
    return GAME_INSTALL_DIRECTORY_PREPROC;
#endif
}

std::filesystem::path BuildUserPath()
{
    if (CheckPortable())
        return g_executableRoot;

    std::filesystem::path userPath;

#if defined(_WIN32)
    PWSTR knownPath = NULL;
    if (SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, NULL, &knownPath) == S_OK)
        userPath = std::filesystem::path{ knownPath } / USER_DIRECTORY;

    CoTaskMemFree(knownPath);
#elif defined(__linux__)
    const char* homeDir = getenv("HOME");
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

const std::filesystem::path& GetUserPath()
{
    return g_userPath;
}
