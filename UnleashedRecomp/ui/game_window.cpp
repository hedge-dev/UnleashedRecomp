#include "game_window.h"
#include "sdl_listener.h"
#include <user/config.h>
#include <SDL_syswm.h>
#include <app.h>
#include <gpu/video.h>

bool m_isFullscreenKeyReleased = true;
bool m_isResizing = false;

int Window_OnSDLEvent(void*, SDL_Event* event)
{
    if (ImGui::GetIO().BackendPlatformUserData != nullptr)
        ImGui_ImplSDL2_ProcessEvent(event);

    for (auto listener : GetEventListeners())
        listener->OnSDLEvent(event);

    switch (event->type)
    {
        case SDL_QUIT:
            App::Exit();
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

                    Config::Fullscreen = GameWindow::SetFullscreen(!GameWindow::IsFullscreen());

                    if (Config::Fullscreen)
                    {
                        Config::Monitor = GameWindow::GetDisplay();
                    }
                    else
                    {
                        Config::WindowState = GameWindow::SetMaximised(Config::WindowState == EWindowState::Maximised);
                    }

                    // Block holding ALT+ENTER spamming window changes.
                    m_isFullscreenKeyReleased = false;

                    break;
                }

                // Restore original window dimensions on F2.
                case SDLK_F2:
                    Config::Fullscreen = GameWindow::SetFullscreen(false);
                    GameWindow::ResetDimensions();
                    break;

                // Recentre window on F3.
                case SDLK_F3:
                {
                    if (GameWindow::IsFullscreen())
                        break;

                    GameWindow::SetDimensions(GameWindow::s_width, GameWindow::s_height);

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
                    GameWindow::s_isFocused = false;
                    SDL_ShowCursor(SDL_ENABLE);
                    break;

                case SDL_WINDOWEVENT_FOCUS_GAINED:
                {
                    GameWindow::s_isFocused = true;

                    if (GameWindow::IsFullscreen())
                        SDL_ShowCursor(GameWindow::s_isFullscreenCursorVisible ? SDL_ENABLE : SDL_DISABLE);

                    break;
                }

                case SDL_WINDOWEVENT_RESTORED:
                    Config::WindowState = EWindowState::Normal;
                    break;

                case SDL_WINDOWEVENT_MAXIMIZED:
                    Config::WindowState = EWindowState::Maximised;
                    break;

                case SDL_WINDOWEVENT_RESIZED:
                    m_isResizing = true;
                    GameWindow::s_width = event->window.data1;
                    GameWindow::s_height = event->window.data2;
                    GameWindow::SetTitle(fmt::format("{} - [{}x{}]", GameWindow::GetTitle(), GameWindow::s_width, GameWindow::s_height).c_str());
                    break;

                case SDL_WINDOWEVENT_MOVED:
                    GameWindow::s_x = event->window.data1;
                    GameWindow::s_y = event->window.data2;
                    break;
            }

            break;
        }

        case SDL_USER_EVILSONIC:
            GameWindow::s_isIconNight = event->user.code;
            GameWindow::SetIcon(GameWindow::s_isIconNight);
            break;
    }

    if (!GameWindow::IsFullscreen())
    {
        if (event->type == SDL_CONTROLLERBUTTONDOWN || event->type == SDL_CONTROLLERBUTTONUP || event->type == SDL_CONTROLLERAXISMOTION)
        {
            // Hide mouse cursor when controller input is detected.
            SDL_ShowCursor(SDL_DISABLE);
        }
        else if (event->type == SDL_MOUSEMOTION)
        {
            // Restore mouse cursor when mouse input is detected.
            SDL_ShowCursor(SDL_ENABLE);
        }
    }

    return 0;
}

void GameWindow::Init(const char *sdlVideoDriver)
{
#ifdef __linux__
    SDL_SetHint("SDL_APP_ID", "io.github.hedge_dev.unleashedrecomp");
#endif

    int videoRes = SDL_VideoInit(sdlVideoDriver);
    if (videoRes != 0 && sdlVideoDriver != nullptr)
    {
        fmt::println("Failed to initialize the specified SDL Video Driver {}. Falling back to default.", sdlVideoDriver);
        SDL_VideoInit(nullptr);
    }

    const char* videoDriverName = SDL_GetCurrentVideoDriver();
    if (videoDriverName != nullptr)
        fmt::println("SDL Video Driver: {}", videoDriverName);

    SDL_EventState(SDL_SYSWMEVENT, SDL_ENABLE);
    SDL_AddEventWatch(Window_OnSDLEvent, s_pWindow);
#ifdef _WIN32
    SetProcessDPIAware();
#endif

    s_x = Config::WindowX;
    s_y = Config::WindowY;
    s_width = Config::WindowWidth;
    s_height = Config::WindowHeight;

    if (s_x == -1 && s_y == -1)
        s_x = s_y = SDL_WINDOWPOS_CENTERED;

    if (!IsPositionValid())
        GameWindow::ResetDimensions();

    s_pWindow = SDL_CreateWindow("SWA", s_x, s_y, s_width, s_height, GetWindowFlags());

    if (IsFullscreen())
        SDL_ShowCursor(SDL_DISABLE);

    SetDisplay(Config::Monitor);

    SetIcon();
    SetTitle();
    SDL_SetWindowMinimumSize(s_pWindow, 640, 480);

    SDL_SysWMinfo info;
    SDL_VERSION(&info.version);
    SDL_GetWindowWMInfo(s_pWindow, &info);

#if defined(_WIN32)
    s_renderWindow = info.info.win.window;
    SetDarkTitleBar(true);
#elif defined(SDL_VULKAN_ENABLED)
    s_renderWindow = s_pWindow;
#elif defined(__linux__)
    s_renderWindow = { info.info.x11.display, info.info.x11.window };
#else
    static_assert(false, "Unknown platform.");
#endif

    SDL_ShowWindow(s_pWindow);
}

void GameWindow::Update()
{
    if (!GameWindow::IsFullscreen() && !GameWindow::IsMaximised() && !s_isChangingDisplay)
    {
        Config::WindowX = GameWindow::s_x;
        Config::WindowY = GameWindow::s_y;
        Config::WindowWidth = GameWindow::s_width;
        Config::WindowHeight = GameWindow::s_height;
    }

    if (m_isResizing)
    {
        SetTitle();
        m_isResizing = false;
    }

    if (g_needsResize)
        s_isChangingDisplay = false;
}
