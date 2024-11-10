#pragma once

#include "kernel/memory.h"
#include "ui/window_listener.h"

class FrontendListener : public WindowListener
{
private:
    bool m_isF8KeyDown = false;

public:
    void OnKeyDown(SDL_Keycode key) override
    {
        if (key == SDLK_F8 && !m_isF8KeyDown)
        {
            // アプリケーション設定 / 開発用 / デバッグ / HUD / 全 HUD 描画
            const auto ms_IsRenderHud = (bool*)g_memory.Translate(0x8328BB26);

            *ms_IsRenderHud = !*ms_IsRenderHud; // Toggle the entire HUD

            printf("[*] HUD %s\n", *ms_IsRenderHud ? "On" : "Off");

            m_isF8KeyDown = true;
        }
    }

    void OnKeyUp(SDL_Keycode key) override
    {
        m_isF8KeyDown = key != SDLK_F8;
    }
};
