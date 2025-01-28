#include <os/registry.h>
#include <unordered_map>

inline const wchar_t* g_registryRoot = L"Software\\UnleashedRecomp";

inline bool os::registry::Init()
{
    return true;
}

template<typename T>
bool os::registry::ReadValue(const std::string_view& name, T& data)
{
    HKEY hKey;

    if (RegOpenKeyExW(HKEY_CURRENT_USER, g_registryRoot, 0, KEY_READ, &hKey) != ERROR_SUCCESS)
        return false;

    wchar_t wideName[128];
    int wideNameSize = MultiByteToWideChar(CP_UTF8, 0, name.data(), name.size(), wideName, sizeof(wideName));
    if (wideNameSize == 0)
    {
        return false;
    }

    wideName[wideNameSize] = 0;
    DWORD bufferSize = 0;
    DWORD dataType = 0;

    auto result = RegGetValueW(hKey, nullptr, wideName, RRF_RT_ANY, &dataType, nullptr, &bufferSize);

    if (result != ERROR_SUCCESS)
    {
        RegCloseKey(hKey);
        return false;
    }

    result = ERROR_INVALID_FUNCTION;
    if constexpr (std::is_same_v<T, std::string>)
    {
        if (dataType == REG_SZ)
        {
            std::vector<uint8_t> buffer{};
            buffer.reserve(bufferSize);
            result = RegGetValueW(hKey, nullptr, wideName, RRF_RT_REG_SZ, nullptr, buffer.data(), &bufferSize);

            if (result == ERROR_SUCCESS)
            {
                int valueSize = WideCharToMultiByte(CP_UTF8, 0, (wchar_t*)buffer.data(), (bufferSize / sizeof(wchar_t)) - 1, nullptr, 0, nullptr, nullptr);
                data.resize(valueSize);
                WideCharToMultiByte(CP_UTF8, 0, (wchar_t*)buffer.data(), (bufferSize / sizeof(wchar_t)) - 1, data.data(), valueSize, nullptr, nullptr);
            }
        }
    }
    else if constexpr (std::is_same_v<T, std::filesystem::path>)
    {
        if (dataType == REG_SZ)
        {
            std::vector<uint8_t> buffer{};
            buffer.reserve(bufferSize);
            result = RegGetValueW(hKey, nullptr, wideName, RRF_RT_REG_SZ, nullptr, buffer.data(), &bufferSize);

            if (result == ERROR_SUCCESS)
            {
                data = reinterpret_cast<wchar_t*>(buffer.data());
            }
        }
    }
    else if constexpr (std::is_same_v<T, uint32_t>)
    {
        result = RegGetValueW(hKey, nullptr, wideName, RRF_RT_DWORD, nullptr, (BYTE*)&data, &bufferSize);
    }
    else if constexpr (std::is_same_v<T, uint64_t>)
    {
        result = RegGetValueW(hKey, nullptr, wideName, RRF_RT_QWORD, nullptr, (BYTE*)&data, &bufferSize);
    }
    else
    {
        static_assert(false, "Unsupported data type.");
    }

    RegCloseKey(hKey);
    return result == ERROR_SUCCESS;
}

template<typename T>
bool os::registry::WriteValue(const std::string_view& name, const T& data)
{
    HKEY hKey;

    if (RegCreateKeyExW(HKEY_CURRENT_USER, g_registryRoot, 0, nullptr, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, NULL) != ERROR_SUCCESS)
        return false;

    BYTE* pData = nullptr;
    DWORD dataSize = 0;
    DWORD dataType = 0;
    bool wideString = false;

    if constexpr (std::is_same_v<T, std::string>)
    {
        pData = (BYTE*)data.c_str();
        dataSize = data.size() + 1;
        dataType = REG_SZ;
    }
    else if constexpr (std::is_same_v<T, uint32_t>)
    {
        pData = &data;
        dataSize = sizeof(T);
        dataType = REG_DWORD;
    }
    else if constexpr (std::is_same_v<T, uint64_t>)
    {
        pData = &data;
        dataSize = sizeof(T);
        dataType = REG_QWORD;
    }
    else if constexpr (std::is_same_v<T, std::filesystem::path>)
    {
        pData = (BYTE*)data.c_str();
        dataSize = (wcslen((const wchar_t*)pData) + 1) * sizeof(wchar_t);
        dataType = REG_SZ;
        wideString = true;
    }
    else
    {
        static_assert(false, "Unsupported data type.");
    }

    LSTATUS result = ERROR_INVALID_FUNCTION;
    if (wideString)
    {
        wchar_t wideName[128];
        int wideNameSize = MultiByteToWideChar(CP_UTF8, 0, name.data(), name.size(), wideName, sizeof(wideName));
        if (wideNameSize == 0)
        {
            return false;
        }

        wideName[wideNameSize] = 0;
        result = RegSetValueExW(hKey, wideName, 0, dataType, pData, dataSize);
    }
    else
    {
        result = RegSetValueExA(hKey, name.data(), 0, dataType, pData, dataSize);
    }

    RegCloseKey(hKey);

    if (result != ERROR_SUCCESS)
        return false;

    return true;
}
