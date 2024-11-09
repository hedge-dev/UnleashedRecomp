#pragma once

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
            printf("F8 pressed!\n");
            m_isF8KeyDown = true;
        }
    }

    void OnKeyUp(SDL_Keycode key) override
    {
        m_isF8KeyDown = key != SDLK_F8;
    }
};
