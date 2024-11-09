#pragma once

#include "ui/window.h"

class IWindowListener
{
public:
    virtual ~IWindowListener() = default;
    virtual void OnSDLEvent(SDL_Event* event) = 0;
    virtual void OnKeyDown(SDL_Keycode key) = 0;
    virtual void OnKeyUp(SDL_Keycode key) = 0;
};

class WindowListener : public IWindowListener
{
public:
    WindowListener()
    {
        Window::s_listeners.emplace_back(this);
    }

    void OnSDLEvent(SDL_Event* event) override {}
    void OnKeyDown(SDL_Keycode key) override {}
    void OnKeyUp(SDL_Keycode key) override {}
};
