#include <kernel/platform.h>

#if _WIN32
LIB_FUNCTION(LONG, "ntdll.dll", RtlGetVersion, PRTL_OSVERSIONINFOW);
#endif

PlatformVersion GetPlatformVersion()
{
    auto result = PlatformVersion{};

#if _WIN32
    OSVERSIONINFOEXW osvi = { 0 };
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEXW);

    if (RtlGetVersion((PRTL_OSVERSIONINFOW)&osvi) != 0)
        return result;

    result.Major = osvi.dwMajorVersion;
    result.Minor = osvi.dwMinorVersion;
    result.Build = osvi.dwBuildNumber;
#endif

    return result;
}
