#include "sdl_listener.h"

std::vector<ISDLEventListener*>& GetEventListeners()
{
    static std::vector<ISDLEventListener*> g_eventListeners;
    return g_eventListeners;
}
