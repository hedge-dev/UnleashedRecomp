#include "view_window.h"
#include <api/SWA.h>
#include <gpu/video.h>
#include <kernel/memory.h>
#include <ui/reddog/reddog_controls.h>
#include <ui/reddog/debug_draw.h>
#include <ui/game_window.h>
#include <user/config.h>

static ViewWindow g_window{ "View" };

void ViewWindow::Draw()
{
    if (Begin())
    {
        Reddog::Checkbox("Render FPS", &Config::ShowFPS.Value);
        Reddog::Checkbox("Render Debug Lines", &Reddog::DebugDraw::ms_IsDrawLine);
        Reddog::Checkbox("Render Debug Text", &Reddog::DebugDraw::ms_IsDrawText);
        Reddog::Checkbox("Render HUD (F8)", SWA::SGlobals::ms_IsRenderHud);
        Reddog::Separator();

        // Reddog::Checkbox((const char*)u8"Render Debug Draw", SWA::SGlobals::ms_IsRenderDebugDraw);
        // Reddog::Checkbox((const char*)u8"Render Debug Position Draw", SWA::SGlobals::ms_IsRenderDebugPositionDraw);
        // Reddog::Checkbox((const char*)u8"Render Debug Draw Text", SWA::SGlobals::ms_IsRenderDebugDrawText);

        // Reddog::Separator();

        // Reddog::Checkbox((const char*)g_memory.Translate(0x82031850), SWA::SGlobals::ms_IsRenderHud);
        Reddog::Checkbox((const char*)u8"Render Main Game HUD", SWA::SGlobals::ms_IsRenderGameMainHud);
        Reddog::Checkbox((const char*)u8"Render Pause HUD", SWA::SGlobals::ms_IsRenderHudPause);

        Reddog::Separator();

        Reddog::Checkbox((const char*)u8"Light Field Debug", SWA::SGlobals::ms_LightFieldDebug);
        Reddog::Checkbox((const char*)u8"Draw Light Field Sampling Point", SWA::SGlobals::ms_DrawLightFieldSamplingPoint);
        Reddog::Checkbox((const char*)u8"Ignore Light Field Data", SWA::SGlobals::ms_IgnoreLightFieldData);

        Reddog::Separator();

        Reddog::Checkbox((const char*)u8"Visualize Loaded GI Mip Level", SWA::SGlobals::ms_VisualizeLoadedLevel);

        Reddog::Separator();

        Reddog::Checkbox((const char*)u8"Render Stage Collision", SWA::SGlobals::ms_IsCollisionRender);
        Reddog::Checkbox((const char*)u8"Render Event Collision", SWA::SGlobals::ms_IsTriggerRender);
        Reddog::Checkbox((const char*)u8"Render Rigid Body Collision", SWA::SGlobals::ms_IsObjectCollisionRender);

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
