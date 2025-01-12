#pragma once

#include <ui/sdl_listener.h>

class FrontendListener : public SDLEventListener
{
    bool m_isF8KeyDown = false;

public:
    void OnSDLEvent(SDL_Event* event) override;
};
