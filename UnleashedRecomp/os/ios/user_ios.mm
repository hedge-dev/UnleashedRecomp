#include <os/user.h>

#import <UIKit/UIKit.h>

bool os::user::IsDarkTheme()
{
    if (@available(iOS 13.0, *))
        return UIScreen.mainScreen.traitCollection.userInterfaceStyle == UIUserInterfaceStyleDark;

    return false;
}
