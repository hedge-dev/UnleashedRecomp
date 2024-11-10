#pragma once

#include "ui/window.h"

class ISDLEventListener
{
public:
    virtual ~ISDLEventListener() = default;
    virtual void OnSDLEvent(SDL_Event* event) = 0;
};

class SDLEventListener : public ISDLEventListener
{
public:
    SDLEventListener()
    {
        Window::s_eventListeners.emplace_back(this);
    }

    void OnSDLEvent(SDL_Event* event) override {}
};
