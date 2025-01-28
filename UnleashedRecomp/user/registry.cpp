#include "registry.h"
#include <os/process.h>
#include <os/registry.h>
#include <user/config.h>

void Registry::Load()
{
    os::registry::ReadValue(STR(RootDirectoryPath), RootDirectoryPath);
}

void Registry::Save()
{
    os::registry::WriteValue(STR(ExecutableFilePath), os::process::GetExecutablePath());
}
