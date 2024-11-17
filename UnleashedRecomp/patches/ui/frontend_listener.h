#pragma once

#include "kernel/memory.h"
#include "ui/sdl_listener.h"
#include "ui/options_menu.h"

class FrontendListener : public SDLEventListener
{
    bool m_isF8KeyDown = false;

public:
    void OnSDLEvent(SDL_Event* event) override
    {
        if (OptionsMenu::s_isVisible)
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

                printf("[*] HUD %s\n", *ms_IsRenderHud ? "On" : "Off");

                m_isF8KeyDown = true;

                break;
            }

            case SDL_KEYUP:
                m_isF8KeyDown = event->key.keysym.sym != SDLK_F8;
                break;
        }
    }
};
