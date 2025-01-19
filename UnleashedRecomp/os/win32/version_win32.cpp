#include <os/version.h>

LIB_FUNCTION(LONG, "ntdll.dll", RtlGetVersion, PRTL_OSVERSIONINFOW);

os::version::OSVersion os::version::GetOSVersion()
{
    auto result = os::version::OSVersion{};

    OSVERSIONINFOEXW osvi = { 0 };
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEXW);

    if (RtlGetVersion((PRTL_OSVERSIONINFOW)&osvi) != 0)
        return result;

    result.Major = osvi.dwMajorVersion;
    result.Minor = osvi.dwMinorVersion;
    result.Build = osvi.dwBuildNumber;

    return result;
}
