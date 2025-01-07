#include "counter_window.h"
#include <gpu/video.h>

static CounterWindow g_window;

void CounterWindow::Draw()
{
    if (Begin())
    {
        Video::DrawCounter();
    }
    End();
}
