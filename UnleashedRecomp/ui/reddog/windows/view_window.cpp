#include "view_window.h"
#include <api/SWA.h>
#include <gpu/video.h>
#include <kernel/memory.h>
#include <ui/reddog/debug_draw.h>
#include <ui/reddog/reddog_controls.h>
#include <ui/game_window.h>
#include <user/config.h>

static ViewWindow g_window;

void ViewWindow::Draw()
{
    if (Begin())
    {
        Reddog::Checkbox("Render FPS", &Config::ShowFPS.Value);

        Reddog::Separator();

        Reddog::Checkbox("Render HUD (F8)", SWA::SGlobals::ms_IsRenderHud);
        Reddog::Checkbox("Render Main Game HUD", SWA::SGlobals::ms_IsRenderGameMainHud);
        Reddog::Checkbox("Render Pause HUD", SWA::SGlobals::ms_IsRenderHudPause);

        Reddog::Separator();

        Reddog::Checkbox("Render Debug Draw", SWA::SGlobals::ms_IsRenderDebugDraw);
        Reddog::Checkbox("Render Debug Draw Text", SWA::SGlobals::ms_IsRenderDebugDrawText);
        Reddog::Checkbox("Render Debug Position Draw", SWA::SGlobals::ms_IsRenderDebugPositionDraw);

        Reddog::Separator();

        Reddog::Checkbox("Draw Light Field Sampling Point", SWA::SGlobals::ms_DrawLightFieldSamplingPoint);
        Reddog::Checkbox("Ignore Light Field Data", SWA::SGlobals::ms_IgnoreLightFieldData);
        Reddog::Checkbox("Light Field Debug", SWA::SGlobals::ms_LightFieldDebug);

        Reddog::Separator();

        Reddog::Checkbox("Visualize Loaded GI Mip Level", SWA::SGlobals::ms_VisualizeLoadedLevel);

        Reddog::Separator();

        Reddog::Checkbox("Render Event Collision", SWA::SGlobals::ms_IsTriggerRender);
        Reddog::Checkbox("Render Object Collision", SWA::SGlobals::ms_IsObjectCollisionRender);
        Reddog::Checkbox("Render Stage Collision", SWA::SGlobals::ms_IsCollisionRender);
        ImGui::TextColored({ 1, 0, 0, 1 }, "* Requires stage restart.");

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
