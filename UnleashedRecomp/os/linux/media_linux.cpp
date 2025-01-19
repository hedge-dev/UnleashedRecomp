#include <os/media.h>

bool os::media::IsExternalMediaPlaying()
{
    // This functionality is not supported in Linux.
    return false;
}
