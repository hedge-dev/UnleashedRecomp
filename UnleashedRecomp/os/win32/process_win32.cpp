#include <os/process.h>

std::filesystem::path os::process::GetExecutablePath()
{
    WCHAR exePath[MAX_PATH];

    if (!GetModuleFileNameW(nullptr, exePath, MAX_PATH))
        return std::filesystem::path();

    return std::filesystem::path(exePath);
}

std::filesystem::path os::process::GetWorkingDirectory()
{
    WCHAR workPath[MAX_PATH];

    if (!GetCurrentDirectoryW(MAX_PATH, workPath))
        return std::filesystem::path();

    return std::filesystem::path(workPath);
}

bool os::process::StartProcess(const std::filesystem::path& path, const std::vector<std::string>& args, std::filesystem::path work)
{
    if (path.empty())
        return false;

    if (work.empty())
        work = path.parent_path();

    auto cli = path.wstring();

    // NOTE: We assume the CLI arguments only contain ASCII characters.
    for (auto& arg : args)
        cli += L" " + std::wstring(arg.begin(), arg.end());

    STARTUPINFOW startInfo{ sizeof(STARTUPINFOW) };
    PROCESS_INFORMATION procInfo{};
    std::wstring pathW = path.wstring();
    std::wstring workW = work.wstring();
    if (!CreateProcessW(pathW.c_str(), cli.data(), nullptr, nullptr, false, 0, nullptr, workW.c_str(), &startInfo, &procInfo))
        return false;

    CloseHandle(procInfo.hProcess);
    CloseHandle(procInfo.hThread);

    return true;
}
