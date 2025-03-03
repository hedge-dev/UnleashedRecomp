#include <os/process.h>

#include <CoreFoundation/CFBundle.h>
#include <dlfcn.h>
#include <mach-o/dyld.h>
#include <signal.h>
#include <sys/param.h>
#include <unistd.h>

std::filesystem::path os::process::GetExecutablePath()
{
    uint32_t exePathSize = PATH_MAX;
    char exePath[PATH_MAX] = {};
    if (_NSGetExecutablePath(exePath, &exePathSize) == 0)
    {
        return std::filesystem::path(std::u8string_view((const char8_t*)(exePath)));
    }
    else
    {
        return std::filesystem::path();
    }
}

std::filesystem::path os::process::GetExecutableRoot()
{
    std::filesystem::path resultPath = GetExecutablePath().remove_filename();
    if (CFBundleRef bundleRef = CFBundleGetMainBundle())
    {
        if (CFURLRef bundleUrlRef = CFBundleCopyBundleURL(bundleRef))
        {
            char appBundlePath[MAXPATHLEN];
            if (CFURLGetFileSystemRepresentation(bundleUrlRef, true, (uint8_t*)(appBundlePath), sizeof(appBundlePath)))
            {
                resultPath = std::filesystem::path(appBundlePath).parent_path();
            }
            CFRelease(bundleUrlRef);
        }
    }
    return resultPath;
}

std::filesystem::path os::process::GetWorkingDirectory()
{
    char cwd[PATH_MAX] = {};
    char *res = getcwd(cwd, sizeof(cwd));
    if (res != nullptr)
    {
        return std::filesystem::path(std::u8string_view((const char8_t*)(cwd)));
    }
    else
    {
        return std::filesystem::path();
    }
}

bool os::process::SetWorkingDirectory(const std::filesystem::path& path)
{
    return chdir(path.c_str()) == 0;
}

bool os::process::StartProcess(const std::filesystem::path& path, const std::vector<std::string>& args, std::filesystem::path work)
{
    pid_t pid = fork();
    if (pid < 0)
        return false;

    if (pid == 0)
    {
        setsid();

        std::u8string workU8 = work.u8string();
        chdir((const char*)(workU8.c_str()));

        std::u8string pathU8 = path.u8string();
        std::vector<char*> argStrs;
        argStrs.push_back((char*)(pathU8.c_str()));
        for (const std::string& arg : args)
            argStrs.push_back((char *)(arg.c_str()));

        argStrs.push_back(nullptr);
        execvp((const char*)(pathU8.c_str()), argStrs.data());
        raise(SIGKILL);
    }

    return true;
}

void os::process::CheckConsole()
{
    // Always visible on macOS.
    g_consoleVisible = true;
}

void os::process::ShowConsole()
{
    // Unnecessary on macOS.
}
