#include <os/process.h>

#import <Foundation/Foundation.h>

#include <mach-o/dyld.h>
#include <sys/param.h>
#include <unistd.h>

std::filesystem::path os::process::GetExecutablePath()
{
    uint32_t exePathSize = PATH_MAX;
    char exePath[PATH_MAX] = {};
    if (_NSGetExecutablePath(exePath, &exePathSize) == 0)
    {
        return std::filesystem::path(std::u8string_view(reinterpret_cast<const char8_t*>(exePath)));
    }

    return {};
}

std::filesystem::path os::process::GetExecutableRoot()
{
    @autoreleasepool
    {
        NSString* resourcePath = [[NSBundle mainBundle] resourcePath];
        if (resourcePath.length > 0)
            return std::filesystem::path(std::u8string_view(reinterpret_cast<const char8_t*>(resourcePath.UTF8String)));
    }

    return GetExecutablePath().remove_filename();
}

std::filesystem::path os::process::GetWorkingDirectory()
{
    char cwd[PATH_MAX] = {};
    if (getcwd(cwd, sizeof(cwd)) != nullptr)
        return std::filesystem::path(std::u8string_view(reinterpret_cast<const char8_t*>(cwd)));

    return {};
}

bool os::process::SetWorkingDirectory(const std::filesystem::path& path)
{
    return chdir(path.c_str()) == 0;
}

bool os::process::StartProcess(const std::filesystem::path& path, const std::vector<std::string>& args, std::filesystem::path work)
{
    (void)path;
    (void)args;
    (void)work;
    return false;
}

void os::process::CheckConsole()
{
    g_consoleVisible = false;
}

void os::process::ShowConsole()
{
}
