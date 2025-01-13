#include "welcome_window.h"

static WelcomeWindow g_window;

void WelcomeWindow::Draw()
{
    if (Begin())
    {
        ImGui::TextColored({ 1, 0, 0, 1 }, "!!! ATTENTION !!!");
        ImGui::Text("For development use only.\nAny changes made here may cause unexpected behaviour.\nUse at your own risk!");
    }
    End();
}
