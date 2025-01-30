#include <kernel/memory.h>
#include <ui/options_menu.h>
#include <os/logger.h>
#include <user/config.h>
#include <sdl_listener.h>

static class FrontendListener : public SDLEventListener
{
    bool m_isF8KeyDown = false;

public:
    void OnSDLEvent(SDL_Event* event) override
    {
        if (!Config::HUDToggleHotkey || OptionsMenu::s_isVisible)
            return;

        switch (event->type)
        {
        case SDL_KEYDOWN:
        {
            if (event->key.keysym.sym != SDLK_F8 || m_isF8KeyDown)
                break;

            // アプリケーション設定 / 開発用 / デバッグ / HUD / 全 HUD 描画
            const auto ms_IsRenderHud = (bool*)g_memory.Translate(0x8328BB26);

            *ms_IsRenderHud = !*ms_IsRenderHud;

            LOGFN("HUD {}", *ms_IsRenderHud ? "ON" : "OFF");

            m_isF8KeyDown = true;

            break;
        }

        case SDL_KEYUP:
            m_isF8KeyDown = event->key.keysym.sym != SDLK_F8;
            break;
        }
    }
} g_frontendlistener;
