#include "registry.h"
#include <os/process.h>
#include <os/registry.h>
#include <user/config.h>

void Registry::Load()
{

}

void Registry::Save()
{
    os::registry::WriteValue(STR(ExecutableFilePath), os::process::GetExecutablePath());
}
