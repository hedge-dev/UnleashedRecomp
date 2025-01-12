#include "frontend_listener.h"
#include <api/SWA.h>
#include <kernel/memory.h>
#include <os/logger.h>
#include <ui/options_menu.h>

FrontendListener g_frontendListener;

void FrontendListener::OnSDLEvent(SDL_Event* event)
{
    if (OptionsMenu::s_isVisible)
        return;

    switch (event->type)
    {
        case SDL_KEYDOWN:
        {
            if (event->key.keysym.sym != SDLK_F8 || m_isF8KeyDown)
                break;

            *SWA::SGlobals::ms_IsRenderHud = !*SWA::SGlobals::ms_IsRenderHud;

            LOGFN("HUD {}", *SWA::SGlobals::ms_IsRenderHud ? "ON" : "OFF");

            m_isF8KeyDown = true;

            break;
        }

        case SDL_KEYUP:
            m_isF8KeyDown = event->key.keysym.sym != SDLK_F8;
            break;
    }
}
