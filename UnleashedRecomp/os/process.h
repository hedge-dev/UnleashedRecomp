#pragma once

namespace os::process
{
    inline bool g_consoleVisible;

    std::filesystem::path GetExecutablePath();
    std::filesystem::path GetWorkingDirectory();
    bool SetWorkingDirectory(const std::filesystem::path& path);
    bool StartProcess(const std::filesystem::path& path, const std::vector<std::string>& args, std::filesystem::path work = {});
    void CheckConsole();
    void ShowConsole();
}
