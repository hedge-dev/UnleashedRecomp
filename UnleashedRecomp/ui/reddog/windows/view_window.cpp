#include "view_window.h"
#include <kernel/memory.h>
#include <ui/reddog/reddog_controls.h>
#include <ui/game_window.h>
#include <user/config.h>

static ViewWindow g_window{ "View" };

void ViewWindow::Draw()
{
    if (Begin())
    {
        Reddog::Checkbox("Draw HUD (F8)", (bool*)g_memory.Translate(0x8328BB26));
        Reddog::Separator();

        if (Reddog::Button("Reset Window Dimensions (F2)"))
        {
            Config::Fullscreen = GameWindow::SetFullscreen(false);
            GameWindow::ResetDimensions();
        }

        if (Reddog::Button("Recentre Window (F3)"))
        {
            if (!GameWindow::IsFullscreen())
                GameWindow::SetDimensions(GameWindow::s_width, GameWindow::s_height);
        }
    }
    End();
}
