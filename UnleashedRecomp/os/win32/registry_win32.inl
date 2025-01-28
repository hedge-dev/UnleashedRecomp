#include <os/registry.h>
#include <unordered_map>

static const std::unordered_map<std::string, HKEY> g_rootKeys =
{
    { "HKEY_CLASSES_ROOT",   HKEY_CLASSES_ROOT   },
    { "HKEY_CURRENT_USER",   HKEY_CURRENT_USER   },
    { "HKEY_LOCAL_MACHINE",  HKEY_LOCAL_MACHINE  },
    { "HKEY_USERS",          HKEY_USERS          },
    { "HKEY_CURRENT_CONFIG", HKEY_CURRENT_CONFIG }
};

static HKEY ParseRootKey(const std::string& name)
{
    auto it = g_rootKeys.find(name);

    if (it == g_rootKeys.end())
        return nullptr;

    return it->second;
}

template<typename T>
bool os::registry::ReadValue(const std::filesystem::path& path, const std::string& name, T& data)
{
    auto pathStr = path.string();
    auto pathSeparator = pathStr.find('\\');

    if (pathSeparator == std::string::npos)
        return false;

    auto rootKey = pathStr.substr(0, pathSeparator);
    auto subKey = pathStr.substr(pathSeparator + 1);

    HKEY hRootKey = ParseRootKey(rootKey);

    if (!hRootKey)
        return false;

    HKEY hKey;

    if (RegOpenKeyExA(hRootKey, subKey.c_str(), 0, KEY_READ, &hKey) != ERROR_SUCCESS)
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
bool os::registry::WriteValue(const std::filesystem::path& path, const std::string& name, const T& data)
{
    auto pathStr = path.string();
    auto pathSeparator = pathStr.find('\\');

    if (pathSeparator == std::string::npos)
        return false;

    auto rootKey = pathStr.substr(0, pathSeparator);
    auto subKey = pathStr.substr(pathSeparator + 1);

    HKEY hRootKey = ParseRootKey(rootKey);

    if (!hRootKey)
        return false;

    HKEY hKey;

    if (RegCreateKeyExA(hRootKey, subKey.c_str(), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, NULL) != ERROR_SUCCESS)
        return false;

    BYTE* pData = nullptr;
    DWORD dataSize = 0;
    DWORD dataType = 0;

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
    else
    {
        static_assert(false, "Unsupported data type.");
    }

    auto result = RegSetValueExA(hKey, name.c_str(), 0, dataType, (const BYTE*)pData, dataSize);

    RegCloseKey(hKey);

    if (result != ERROR_SUCCESS)
        return false;

    return true;
}
