#include <os/process_detail.h>

std::filesystem::path os::process::detail::GetExecutablePath()
{
    assert(false && "Unimplemented.");
    return std::filesystem::path();
}

std::filesystem::path os::process::detail::GetWorkingDirectory()
{
    // TODO: There's a cross platform way to retrieve this which is just getting it from argv[0].
    assert(false && "Unimplemented.");
    return std::filesystem::path();
}

bool os::process::detail::StartProcess(const std::filesystem::path path, const std::vector<std::string> args, std::filesystem::path work)
{
    assert(false && "Unimplemented.");
    return false;
}
