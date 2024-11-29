#include "window.h"
#include "sdl_listener.h"
#include <cfg/config.h>
#include <SDL_syswm.h>

bool m_isFullscreenKeyReleased = true;

int Window_OnSDLEvent(void*, SDL_Event* event)
{
    if (ImGui::GetIO().BackendPlatformUserData != nullptr)
        ImGui_ImplSDL2_ProcessEvent(event);

    for (auto listener : Window::s_eventListeners)
        listener->OnSDLEvent(event);

    switch (event->type)
    {
        case SDL_QUIT:
            Config::Save();
            ExitProcess(0);
            break;

        case SDL_KEYDOWN:
        {
            switch (event->key.keysym.sym)
            {
                // Toggle fullscreen on ALT+ENTER.
                case SDLK_RETURN:
                {
                    if (!(event->key.keysym.mod & KMOD_ALT) || !m_isFullscreenKeyReleased)
                        break;

                    Config::Fullscreen = Window::SetFullscreen(!Window::IsFullscreen());

                    if (!Config::Fullscreen)
                        Config::WindowState = Window::SetMaximised(Config::WindowState == EWindowState::Maximised);

                    // Block holding ALT+ENTER spamming window changes.
                    m_isFullscreenKeyReleased = false;

                    break;
                }

                // Restore original window dimensions on F2.
                case SDLK_F2:
                    Config::Fullscreen = Window::SetFullscreen(false);
                    Window::SetDimensions(DEFAULT_WIDTH, DEFAULT_HEIGHT);
                    break;

                // Recentre window on F3.
                case SDLK_F3:
                {
                    if (Window::IsFullscreen())
                        break;

                    Window::SetDimensions(Window::s_width, Window::s_height);

                    break;
                }
            }

            break;
        }

        case SDL_KEYUP:
        {
            switch (event->key.keysym.sym)
            {
                // Allow user to input ALT+ENTER again.
                case SDLK_RETURN:
                    m_isFullscreenKeyReleased = true;
                    break;
            }
        }

        case SDL_WINDOWEVENT:
        {
            switch (event->window.event)
            {
                case SDL_WINDOWEVENT_FOCUS_LOST:
                    Window::s_isFocused = false;
                    SDL_ShowCursor(SDL_ENABLE);
                    break;

                case SDL_WINDOWEVENT_FOCUS_GAINED:
                    Window::s_isFocused = true;
                    SDL_ShowCursor(Window::IsFullscreen() ? SDL_DISABLE : SDL_ENABLE);
                    break;

                case SDL_WINDOWEVENT_RESTORED:
                    Config::WindowState = EWindowState::Normal;
                    break;

                case SDL_WINDOWEVENT_MAXIMIZED:
                    Config::WindowState = EWindowState::Maximised;
                    break;

                case SDL_WINDOWEVENT_RESIZED:
                    Window::s_width = event->window.data1;
                    Window::s_height = event->window.data2;
                    break;

                case SDL_WINDOWEVENT_MOVED:
                    Window::s_x = event->window.data1;
                    Window::s_y = event->window.data2;
                    break;
            }

            break;
        }

        case SDL_USER_EVILSONIC:
        {
            Window::s_isIconNight = event->user.code;
            Window::SetIcon(Window::s_isIconNight);
            break;
        }
    }

    return 0;
}

void Window::Init()
{
    SDL_InitSubSystem(SDL_INIT_VIDEO);
    SDL_EventState(SDL_SYSWMEVENT, SDL_ENABLE);
    SDL_AddEventWatch(Window_OnSDLEvent, s_pWindow);
    SetProcessDPIAware();

    s_x = Config::WindowX;
    s_y = Config::WindowY;
    s_width = Config::WindowWidth;
    s_height = Config::WindowHeight;

    if (s_x == -1 && s_y == -1)
        s_x = s_y = SDL_WINDOWPOS_CENTERED;

    if (!IsPositionValid())
    {
        s_x = SDL_WINDOWPOS_CENTERED;
        s_y = SDL_WINDOWPOS_CENTERED;
        s_width = DEFAULT_WIDTH;
        s_height = DEFAULT_HEIGHT;

        Config::WindowX = s_x;
        Config::WindowY = s_y;
        Config::WindowWidth = s_width;
        Config::WindowHeight = s_height;
    }

    s_pWindow = SDL_CreateWindow("SWA", s_x, s_y, s_width, s_height, GetWindowFlags());

    if (IsFullscreen())
        SDL_ShowCursor(SDL_DISABLE);

    SetIcon();
    SetTitle();
    SDL_SetWindowMinimumSize(s_pWindow, 640, 480);

    SDL_SysWMinfo info;
    SDL_VERSION(&info.version);
    SDL_GetWindowWMInfo(s_pWindow, &info);

    s_handle = info.info.win.window;
}

void Window::Update()
{
    if (!Window::IsFullscreen() && !Window::IsMaximised())
    {
        Config::WindowX = Window::s_x;
        Config::WindowY = Window::s_y;
        Config::WindowWidth = Window::s_width;
        Config::WindowHeight = Window::s_height;
    }
}
