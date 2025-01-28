#pragma once

namespace os::registry
{
    template<typename T>
    bool ReadValue(const std::filesystem::path& path, const std::string& name, T& data);

    template<typename T>
    bool WriteValue(const std::filesystem::path& path, const std::string& name, const T& data);
}

#if _WIN32
#include <os/win32/registry_win32.inl>
#elif defined(__linux__)
#include <os/linux/registry_linux.inl>
#endif
