#include <os/user.h>

bool os::user::IsDarkTheme()
{
    HKEY hKey;

    if (RegOpenKeyExA(HKEY_CURRENT_USER, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize", 0, KEY_READ, &hKey) == ERROR_SUCCESS)
    {
        DWORD value = 0;
        DWORD valueSize = sizeof(value);

        if (RegQueryValueExA(hKey, "AppsUseLightTheme", nullptr, nullptr, (LPBYTE)&value, &valueSize) == ERROR_SUCCESS)
        {
            RegCloseKey(hKey);

            return value == 0;
        }

        RegCloseKey(hKey);
    }

    return false;
}
