#pragma once

namespace os::registry
{
    bool Init();

    template<typename T>
    bool ReadValue(const std::string_view& name, T& data);

    template<typename T>
    bool WriteValue(const std::string_view& name, const T& data);
}

#if _WIN32
#include <os/win32/registry_win32.inl>
#elif defined(__linux__)
#include <os/linux/registry_linux.inl>
#elif defined(__APPLE__)
#include <os/macos/registry_macos.inl>
#endif
