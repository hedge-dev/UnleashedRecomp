#pragma once

#include <SDL.h>
#include "ui/window.h"

#define SDL_USER_EVILSONIC (SDL_USEREVENT + 1)

inline static void SDL_ResizeEvent(SDL_Window* pWindow, int width, int height)
{
    SDL_Event event{};
    event.type = SDL_WINDOWEVENT;
    event.window.event = SDL_WINDOWEVENT_RESIZED;
    event.window.windowID = SDL_GetWindowID(pWindow);
    event.window.data1 = width;
    event.window.data2 = height;

    SDL_PushEvent(&event);
}

inline static void SDL_MoveEvent(SDL_Window* pWindow, int x, int y)
{
    SDL_Event event{};
    event.type = SDL_WINDOWEVENT;
    event.window.event = SDL_WINDOWEVENT_MOVED;
    event.window.windowID = SDL_GetWindowID(pWindow);
    event.window.data1 = x;
    event.window.data2 = y;

    SDL_PushEvent(&event);
}

inline static void SDL_User_EvilSonic(SDL_Window* pWindow, bool isCtor)
{
    SDL_Event event{};
    event.type = SDL_USER_EVILSONIC;
    event.user.code = isCtor;

    SDL_PushEvent(&event);
}
