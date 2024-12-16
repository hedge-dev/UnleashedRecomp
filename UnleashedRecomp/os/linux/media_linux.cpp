#include <os/media_detail.h>

bool os::media::detail::IsExternalMediaPlaying()
{
    // This functionality is not supported in Linux.
    return false;
}
