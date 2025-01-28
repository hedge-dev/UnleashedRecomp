#include <os/registry.h>
#include <unordered_map>

inline const wchar_t* g_registryRoot = L"Software\\UnleashedRecomp";

inline bool os::registry::Init()
{
    return true;
}

template<typename T>
bool os::registry::ReadValue(const std::string& name, T& data)
{
    HKEY hKey;

    if (RegOpenKeyExW(HKEY_CURRENT_USER, g_registryRoot, 0, KEY_READ, &hKey) != ERROR_SUCCESS)
        return false;

    BYTE buffer[512];
    DWORD bufferSize = sizeof(buffer);
    DWORD dataType = 0;

    auto result = RegQueryValueExA(hKey, name.c_str(), NULL, &dataType, buffer, &bufferSize);

    RegCloseKey(hKey);

    if (result != ERROR_SUCCESS)
        return false;

    if constexpr (std::is_same_v<T, std::string>)
    {
        if (dataType != REG_SZ)
            return false;

        data = std::string((char*)buffer, bufferSize - 1);
    }
    else if constexpr (std::is_same_v<T, uint32_t>)
    {
        if (dataType != REG_DWORD)
            return false;

        data = *(uint32_t*)buffer;
    }
    else if constexpr (std::is_same_v<T, uint64_t>)
    {
        if (dataType != REG_QWORD)
            return false;

        data = *(uint32_t*)buffer;
    }
    else
    {
        static_assert(false, "Unsupported data type.");
    }

    return true;
}

template<typename T>
bool os::registry::WriteValue(const std::string& name, const T& data)
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
        int wideNameSize = MultiByteToWideChar(CP_UTF8, 0, name.c_str(), name.size(), wideName, sizeof(wideName));
        if (wideNameSize == 0)
        {
            return false;
        }

        wideName[wideNameSize] = 0;
        result = RegSetValueExW(hKey, wideName, 0, dataType, pData, dataSize);
    }
    else
    {
        result = RegSetValueExA(hKey, name.c_str(), 0, dataType, pData, dataSize);
    }

    RegCloseKey(hKey);

    if (result != ERROR_SUCCESS)
        return false;

    return true;
}
