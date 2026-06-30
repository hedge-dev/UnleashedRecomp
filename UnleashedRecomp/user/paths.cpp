#include "paths.h"
#include <os/process.h>

#if defined(UNLEASHED_RECOMP_IOS)
std::filesystem::path g_executableRoot;
std::filesystem::path g_userPath;

void InitPaths()
{
    if (!g_executableRoot.empty())
        return;

    g_executableRoot = os::process::GetExecutableRoot();
    g_userPath = BuildUserPath();
}
#else
std::filesystem::path g_executableRoot = os::process::GetExecutableRoot();
std::filesystem::path g_userPath = BuildUserPath();

void InitPaths()
{
}
#endif

bool CheckPortable()
{
    return std::filesystem::exists(g_executableRoot / "portable.txt");
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
#elif defined(UNLEASHED_RECOMP_IOS)
    const char* homeDir = getenv("HOME");
    if (homeDir != nullptr)
    {
        userPath = std::filesystem::path(homeDir) / "Documents" / USER_DIRECTORY;
    }
#elif defined(__linux__) || defined(__APPLE__)
    const char* homeDir = getenv("HOME");
#if defined(__linux__)
    if (homeDir == nullptr)
    {
        homeDir = getpwuid(getuid())->pw_dir;
    }
#endif

    if (homeDir != nullptr)
    {
        // Prefer to store in the .config directory if it exists. Use the home directory otherwise.
        std::filesystem::path homePath = homeDir;
#if defined(__linux__)
        std::filesystem::path configPath = homePath / ".config";
#else
        std::filesystem::path configPath = homePath / "Library" / "Application Support";
#endif
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
