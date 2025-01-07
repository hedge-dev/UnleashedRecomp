#include "view_window.h"
#include <kernel/memory.h>
#include <ui/reddog/reddog_controls.h>

static ViewWindow g_window{ "View" };

void ViewWindow::Draw()
{
    if (Begin())
    {
        Reddog::Checkbox("Render HUD", (bool*)g_memory.Translate(0x8328BB26));
    }
    End();
}
