#include <os/version.h>

#import <UIKit/UIKit.h>

os::version::OSVersion os::version::GetOSVersion()
{
    os::version::OSVersion result;

    NSOperatingSystemVersion version = NSProcessInfo.processInfo.operatingSystemVersion;
    result.Major = static_cast<uint32_t>(version.majorVersion);
    result.Minor = static_cast<uint32_t>(version.minorVersion);
    result.Build = static_cast<uint32_t>(version.patchVersion);

    return result;
}
