#include "registry.h"
#include <os/process.h>
#include <os/registry.h>

void Registry::Save()
{
    os::registry::WriteValue(STR(ExecutableFilePath), os::process::GetExecutablePath());
}
