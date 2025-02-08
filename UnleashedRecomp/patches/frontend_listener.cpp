#include <kernel/memory.h>
#include <ui/options_menu.h>
#include <os/logger.h>
#include <user/config.h>
#include <sdl_listener.h>

static class FrontendListener : public SDLEventListener
{
    bool m_isF8KeyDown = false;

public:
    bool OnSDLEvent(SDL_Event* event) override
    {
        if (!Config::HUDToggleKey || OptionsMenu::s_isVisible)
            return false;

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

        return false;
    }
}
g_frontendListener;
