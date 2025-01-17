#include <os/user.h>
#include <os/user_detail.h>

bool os::user::IsDarkTheme()
{
    return detail::IsDarkTheme();
}
