#include "game_window.h"
#include <gpu/video.h>
#include <os/logger.h>
#include <os/user.h>
#include <os/version.h>
#include <app.h>
#include <sdl_listener.h>
#include <SDL_syswm.h>

#if _WIN32
#include <dwmapi.h>
#include <shellscalingapi.h>
#endif

#include <res/images/game_icon.bmp.h>
#include <res/images/game_icon_night.bmp.h>

bool m_isFullscreenKeyReleased = true;
bool m_isResizing = false;

int Window_OnSDLEvent(void*, SDL_Event* event)
{
    if (ImGui::GetIO().BackendPlatformUserData != nullptr)
        ImGui_ImplSDL2_ProcessEvent(event);

    for (auto listener : GetEventListeners())
    {
        if (listener->OnSDLEvent(event))
        {
            return 0;
        }
    }

    switch (event->type)
    {
        case SDL_QUIT:
        {
            if (App::s_isSaving)
                break;

            App::Exit();

            break;
        }

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
                    Config::WindowSize = -1;
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

    return 0;
}

void GameWindow::Init(const char* sdlVideoDriver)
{
#ifdef __linux__
    SDL_SetHint("SDL_APP_ID", "io.github.hedge_dev.unleashedrecomp");
#endif

    if (SDL_VideoInit(sdlVideoDriver) != 0 && sdlVideoDriver)
    {
        LOGFN_ERROR("Failed to initialise the SDL video driver: \"{}\". Falling back to default.", sdlVideoDriver);
        SDL_VideoInit(nullptr);
    }

    auto videoDriverName = SDL_GetCurrentVideoDriver();

    if (videoDriverName)
        LOGFN("SDL video driver: \"{}\"", videoDriverName);

    SDL_EventState(SDL_SYSWMEVENT, SDL_ENABLE);
    SDL_AddEventWatch(Window_OnSDLEvent, s_pWindow);

#ifdef _WIN32
    SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);
#endif

    s_x = Config::WindowX;
    s_y = Config::WindowY;
    s_width = Config::WindowWidth;
    s_height = Config::WindowHeight;

    if (s_x == -1 && s_y == -1)
        s_x = s_y = SDL_WINDOWPOS_CENTERED;

    if (!IsPositionValid())
        GameWindow::ResetDimensions();

    s_pWindow = SDL_CreateWindow("Unleashed Recompiled", s_x, s_y, s_width, s_height, GetWindowFlags());

    if (IsFullscreen())
        SDL_ShowCursor(SDL_DISABLE);

    SetDisplay(Config::Monitor);
    SetIcon();
    SetTitle();

    SDL_SetWindowMinimumSize(s_pWindow, MIN_WIDTH, MIN_HEIGHT);

    SDL_SysWMinfo info;
    SDL_VERSION(&info.version);
    SDL_GetWindowWMInfo(s_pWindow, &info);

#if defined(_WIN32)
    s_renderWindow = info.info.win.window;

    if (Config::DisableDWMRoundedCorners)
    {
        DWM_WINDOW_CORNER_PREFERENCE wcp = DWMWCP_DONOTROUND;
        DwmSetWindowAttribute(s_renderWindow, DWMWA_WINDOW_CORNER_PREFERENCE, &wcp, sizeof(wcp));
    }
#elif defined(SDL_VULKAN_ENABLED)
    s_renderWindow = s_pWindow;
#elif defined(__linux__)
    s_renderWindow = { info.info.x11.display, info.info.x11.window };
#else
    static_assert(false, "Unknown platform.");
#endif

    SetTitleBarColour();

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

SDL_Surface* GameWindow::GetIconSurface(void* pIconBmp, size_t iconSize)
{
    auto rw = SDL_RWFromMem(pIconBmp, iconSize);
    auto surface = SDL_LoadBMP_RW(rw, 1);

    if (!surface)
        LOGF_ERROR("Failed to load icon: {}", SDL_GetError());

    return surface;
}

void GameWindow::SetIcon(void* pIconBmp, size_t iconSize)
{
    if (auto icon = GetIconSurface(pIconBmp, iconSize))
    {
        SDL_SetWindowIcon(s_pWindow, icon);
        SDL_FreeSurface(icon);
    }
}

void GameWindow::SetIcon(bool isNight)
{
    if (isNight)
    {
        SetIcon(g_game_icon_night, sizeof(g_game_icon_night));
    }
    else
    {
        SetIcon(g_game_icon, sizeof(g_game_icon));
    }
}

const char* GameWindow::GetTitle()
{
    if (Config::UseOfficialTitleOnTitleBar)
    {
        auto isSWA = Config::Language == ELanguage::Japanese;

        if (Config::UseAlternateTitle)
            isSWA = !isSWA;

        return isSWA
            ? "SONIC WORLD ADVENTURE"
            : "SONIC UNLEASHED";
    }

    return "Unleashed Recompiled";
}

void GameWindow::SetTitle(const char* title)
{
    SDL_SetWindowTitle(s_pWindow, title ? title : GetTitle());
}

void GameWindow::SetTitleBarColour()
{
#if _WIN32
    if (os::user::IsDarkTheme())
    {
        auto version = os::version::GetOSVersion();

        if (version.Major < 10 || version.Build <= 17763)
            return;

        auto flag = version.Build >= 18985
            ? DWMWA_USE_IMMERSIVE_DARK_MODE
            : 19; // DWMWA_USE_IMMERSIVE_DARK_MODE_BEFORE_20H1

        const DWORD useImmersiveDarkMode = 1;
        DwmSetWindowAttribute(s_renderWindow, flag, &useImmersiveDarkMode, sizeof(useImmersiveDarkMode));
    }
#endif
}

bool GameWindow::IsFullscreen()
{
    return SDL_GetWindowFlags(s_pWindow) & SDL_WINDOW_FULLSCREEN_DESKTOP;
}

bool GameWindow::SetFullscreen(bool isEnabled)
{
    if (isEnabled)
    {
        SDL_SetWindowFullscreen(s_pWindow, SDL_WINDOW_FULLSCREEN_DESKTOP);
        SDL_ShowCursor(s_isFullscreenCursorVisible ? SDL_ENABLE : SDL_DISABLE);
    }
    else
    {
        SDL_SetWindowFullscreen(s_pWindow, 0);
        SDL_ShowCursor(SDL_ENABLE);

        SetIcon(GameWindow::s_isIconNight);
        SetDimensions(Config::WindowWidth, Config::WindowHeight, Config::WindowX, Config::WindowY);
    }

    return isEnabled;
}
    
void GameWindow::SetFullscreenCursorVisibility(bool isVisible)
{
    s_isFullscreenCursorVisible = isVisible;

    if (IsFullscreen())
    {
        SDL_ShowCursor(s_isFullscreenCursorVisible ? SDL_ENABLE : SDL_DISABLE);
    }
    else
    {
        SDL_ShowCursor(SDL_ENABLE);
    }
}

bool GameWindow::IsMaximised()
{
    return SDL_GetWindowFlags(s_pWindow) & SDL_WINDOW_MAXIMIZED;
}

EWindowState GameWindow::SetMaximised(bool isEnabled)
{
    if (isEnabled)
    {
        SDL_MaximizeWindow(s_pWindow);
    }
    else
    {
        SDL_RestoreWindow(s_pWindow);
    }

    return isEnabled
        ? EWindowState::Maximised
        : EWindowState::Normal;
}

SDL_Rect GameWindow::GetDimensions()
{
    SDL_Rect rect{};

    SDL_GetWindowPosition(s_pWindow, &rect.x, &rect.y);
    SDL_GetWindowSize(s_pWindow, &rect.w, &rect.h);

    return rect;
}

void GameWindow::GetSizeInPixels(int *w, int *h)
{
    SDL_GetWindowSizeInPixels(s_pWindow, w, h);
}

void GameWindow::SetDimensions(int w, int h, int x, int y)
{
    s_width = w;
    s_height = h;
    s_x = x;
    s_y = y;

    SDL_SetWindowSize(s_pWindow, w, h);
    SDL_ResizeEvent(s_pWindow, w, h);

    SDL_SetWindowPosition(s_pWindow, x, y);
    SDL_MoveEvent(s_pWindow, x, y);
}

void GameWindow::ResetDimensions()
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

uint32_t GameWindow::GetWindowFlags()
{
    uint32_t flags = SDL_WINDOW_HIDDEN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI;

    if (Config::WindowState == EWindowState::Maximised)
        flags |= SDL_WINDOW_MAXIMIZED;

    if (Config::Fullscreen)
        flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;

#ifdef SDL_VULKAN_ENABLED
    flags |= SDL_WINDOW_VULKAN;
#endif

    return flags;
}

int GameWindow::GetDisplayCount()
{
    auto result = SDL_GetNumVideoDisplays();

    if (result < 0)
    {
        LOGF_ERROR("Failed to get display count: {}", SDL_GetError());
        return 1;
    }

    return result;
}

int GameWindow::GetDisplay()
{
    return SDL_GetWindowDisplayIndex(s_pWindow);
}

void GameWindow::SetDisplay(int displayIndex)
{
    if (!IsFullscreen())
        return;

    if (GetDisplay() == displayIndex)
        return;

    s_isChangingDisplay = true;

    SDL_Rect bounds;

    if (SDL_GetDisplayBounds(displayIndex, &bounds) == 0)
    {
        SetFullscreen(false);
        SetDimensions(bounds.w, bounds.h, bounds.x, bounds.y);
        SetFullscreen(true);
    }
    else
    {
        ResetDimensions();
    }
}

std::vector<SDL_DisplayMode> GameWindow::GetDisplayModes(bool ignoreInvalidModes, bool ignoreRefreshRates)
{
    auto result = std::vector<SDL_DisplayMode>();
    auto uniqueResolutions = std::set<std::pair<int, int>>();
    auto displayIndex = GetDisplay();
    auto modeCount = SDL_GetNumDisplayModes(displayIndex);

    if (modeCount <= 0)
        return result;

    for (int i = modeCount - 1; i >= 0; i--)
    {
        SDL_DisplayMode mode;

        if (SDL_GetDisplayMode(displayIndex, i, &mode) == 0)
        {
            if (ignoreInvalidModes)
            {
                if (mode.w < MIN_WIDTH || mode.h < MIN_HEIGHT)
                    continue;

                SDL_DisplayMode desktopMode;

                if (SDL_GetDesktopDisplayMode(displayIndex, &desktopMode) == 0)
                {
                    if (mode.w >= desktopMode.w || mode.h >= desktopMode.h)
                        continue;
                }
            }

            if (ignoreRefreshRates)
            {
                auto res = std::make_pair(mode.w, mode.h);

                if (uniqueResolutions.find(res) == uniqueResolutions.end())
                {
                    uniqueResolutions.insert(res);
                    result.push_back(mode);
                }
            }
            else
            {
                result.push_back(mode);
            }
        }
    }

    return result;
}

int GameWindow::FindNearestDisplayMode()
{
    auto result = -1;
    auto displayModes = GetDisplayModes();
    auto currentDiff = std::numeric_limits<int>::max();

    for (int i = 0; i < displayModes.size(); i++)
    {
        auto& mode = displayModes[i];

        auto widthDiff = abs(mode.w - s_width);
        auto heightDiff = abs(mode.h - s_height);
        auto totalDiff = widthDiff + heightDiff;

        if (totalDiff < currentDiff)
        {
            currentDiff = totalDiff;
            result = i;
        }
    }

    return result;
}

bool GameWindow::IsPositionValid()
{
    auto displayCount = GetDisplayCount();

    for (int i = 0; i < displayCount; i++)
    {
        SDL_Rect bounds;

        if (SDL_GetDisplayBounds(i, &bounds) == 0)
        {
            auto x = s_x;
            auto y = s_y;

            // Window spans across the entire display in windowed mode, which is invalid.
            if (!Config::Fullscreen && s_width == bounds.w && s_height == bounds.h)
                return false;

            if (x == SDL_WINDOWPOS_CENTERED_DISPLAY(i))
                x = bounds.w / 2 - s_width / 2;

            if (y == SDL_WINDOWPOS_CENTERED_DISPLAY(i))
                y = bounds.h / 2 - s_height / 2;

            if (x >= bounds.x && x < bounds.x + bounds.w &&
                y >= bounds.y && y < bounds.y + bounds.h)
            {
                return true;
            }
        }
    }

    return false;
}
