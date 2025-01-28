#include <os/registry.h>

// TODO: Implement
inline bool os::registry::Init()
{
    return false;
}

// TODO: read from file?
template<typename T>
bool os::registry::ReadValue(const std::filesystem::path& path, const std::string& name, T& data)
{
    return false;
}

// TODO: write to file?
template<typename T>
bool os::registry::WriteValue(const std::filesystem::path& path, const std::string& name, const T& data)
{
    return false;
}
