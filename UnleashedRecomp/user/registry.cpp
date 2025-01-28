#include "registry.h"
#include <os/process.h>
#include <os/registry.h>
#include <user/config.h>

void Registry::Load()
{
    std::filesystem::path path = "HKEY_CURRENT_USER\\Software\\UnleashedRecomp";

    os::registry::ReadValue(path, STR(ExecutableFilePath), ExecutableFilePath);
    os::registry::ReadValue(path, STR(RootDirectoryPath), RootDirectoryPath);
}

void Registry::Save()
{
    std::filesystem::path path = "HKEY_CURRENT_USER\\Software\\UnleashedRecomp";

    ExecutableFilePath = os::process::GetExecutablePath().string();

    os::registry::WriteValue(path, STR(ExecutableFilePath), ExecutableFilePath);
}
