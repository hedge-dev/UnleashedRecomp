#include "view_window.h"
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
        Reddog::Checkbox("Render HUD (F8)", (bool*)g_memory.Translate(0x8328BB26));
        Reddog::Separator();

        //// デバッグ描画
        //Reddog::Checkbox((const char*)u8"Render Debug Draw", (bool*)g_memory.Translate(0x8328BB23));
        //// デバッグ位置描画
        //Reddog::Checkbox((const char*)u8"Render Debug Position Draw", (bool*)g_memory.Translate(0x8328BB24));
        //// デバッグ文字描画
        //Reddog::Checkbox((const char*)u8"Render Debug Draw Text", (bool*)g_memory.Translate(0x8328BB25));

        //Reddog::Separator();

        //// 全 HUD 描画
        //Reddog::Checkbox((const char*)g_memory.Translate(0x82031850), (bool*)g_memory.Translate(0x8328BB26));
        // ゲームメインHUD 描画
        Reddog::Checkbox((const char*)u8"Render Main Game HUD", (bool*)g_memory.Translate(0x8328BB27));
        // ポーズメニュー 描画
        Reddog::Checkbox((const char*)u8"Render Pause HUD", (bool*)g_memory.Translate(0x8328BB28));

        Reddog::Separator();

        // 値をデバッグ表示
        Reddog::Checkbox((const char*)u8"Light Field Debug", (bool*)g_memory.Translate(0x83367BCD));
        // サンプリング点をデバッグ表示
        Reddog::Checkbox((const char*)u8"Draw Light Field Sampling Point", (bool*)g_memory.Translate(0x83367BCE));
        // データを無視する
        Reddog::Checkbox((const char*)u8"Ignore Light Field Data", (bool*)g_memory.Translate(0x83367BCF));

        Reddog::Separator();

        // ミップレベルを視覚化 赤=0, 緑=1, 青=2, 黄=未ロード
        Reddog::Checkbox((const char*)u8"Visualize Loaded GI Mip Level", (bool*)g_memory.Translate(0x833678C1));

        Reddog::Separator();

        // IsCollisionRender
        Reddog::Checkbox((const char*)u8"Render Stage Collision", (bool*)g_memory.Translate(0x833678A6));
        // IsTriggerRender
        Reddog::Checkbox((const char*)u8"Render Event Collision", (bool*)g_memory.Translate(0x83367904));
        // IsObjectCollisionRender
        Reddog::Checkbox((const char*)u8"Render Rigid Body Collision", (bool*)g_memory.Translate(0x83367905));

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
