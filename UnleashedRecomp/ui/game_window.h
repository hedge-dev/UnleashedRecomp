#pragma once

#include <res/images/game_icon.bmp.h>
#include <res/images/game_icon_night.bmp.h>
#include <os/logger.h>
#include <os/version.h>
#include <ui/window_events.h>
#include <user/config.h>
#include <gpu/rhi/plume_render_interface_types.h>

#if _WIN32
#include <dwmapi.h>
#pragma comment(lib, "dwmapi.lib")
#endif

#define DEFAULT_WIDTH 1280
#define DEFAULT_HEIGHT 720

class GameWindow
{
public:
    static inline SDL_Window* s_pWindow;
    static inline plume::RenderWindow s_renderWindow;

    static inline int s_x;
    static inline int s_y;
    static inline int s_width = DEFAULT_WIDTH;
    static inline int s_height = DEFAULT_HEIGHT;

    static inline bool s_isFocused;
    static inline bool s_isIconNight;
    static inline bool s_isFullscreenCursorVisible;
    static inline bool s_isChangingDisplay;

    static SDL_Surface* GetIconSurface(void* pIconBmp, size_t iconSize)
    {
        auto rw = SDL_RWFromMem(pIconBmp, iconSize);
        auto surface = SDL_LoadBMP_RW(rw, 1);

        if (!surface)
            LOGF_ERROR("Failed to load icon: {}", SDL_GetError());

        return surface;
    }

    static void SetIcon(void* pIconBmp, size_t iconSize)
    {
        if (auto icon = GetIconSurface(pIconBmp, iconSize))
        {
            SDL_SetWindowIcon(s_pWindow, icon);
            SDL_FreeSurface(icon);
        }
    }

    static void SetIcon(bool isNight = false)
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

    static const char* GetTitle()
    {
        return Config::Language == ELanguage::Japanese
            ? "SONIC WORLD ADVENTURE"
            : "SONIC UNLEASHED";
    }

    static void SetTitle(const char* title = nullptr)
    {
        SDL_SetWindowTitle(s_pWindow, title ? title : GetTitle());
    }

    static void SetDarkTitleBar(bool isEnabled)
    {
#if _WIN32
        auto version = os::version::GetOSVersion();

        if (version.Major < 10 || version.Build <= 17763)
            return;

        auto flag = version.Build >= 18985
            ? DWMWA_USE_IMMERSIVE_DARK_MODE
            : 19; // DWMWA_USE_IMMERSIVE_DARK_MODE_BEFORE_20H1

        const DWORD useImmersiveDarkMode = isEnabled;
        DwmSetWindowAttribute(s_renderWindow, flag, &useImmersiveDarkMode, sizeof(useImmersiveDarkMode));
#endif
    }

    static bool IsFullscreen()
    {
        return SDL_GetWindowFlags(s_pWindow) & SDL_WINDOW_FULLSCREEN_DESKTOP;
    }

    static bool SetFullscreen(bool isEnabled)
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
    
    static void SetFullscreenCursorVisibility(bool isVisible)
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

    static bool IsMaximised()
    {
        return SDL_GetWindowFlags(s_pWindow) & SDL_WINDOW_MAXIMIZED;
    }

    static EWindowState SetMaximised(bool isEnabled)
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

    static SDL_Rect GetDimensions()
    {
        SDL_Rect rect{};

        SDL_GetWindowPosition(s_pWindow, &rect.x, &rect.y);
        SDL_GetWindowSize(s_pWindow, &rect.w, &rect.h);

        return rect;
    }

    static void SetDimensions(int w, int h, int x = SDL_WINDOWPOS_CENTERED, int y = SDL_WINDOWPOS_CENTERED)
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

    static void ResetDimensions()
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

    static uint32_t GetWindowFlags()
    {
        uint32_t flags = SDL_WINDOW_HIDDEN | SDL_WINDOW_RESIZABLE;

        if (Config::WindowState == EWindowState::Maximised)
            flags |= SDL_WINDOW_MAXIMIZED;

        if (Config::Fullscreen)
            flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;

#ifdef SDL_VULKAN_ENABLED
        flags |= SDL_WINDOW_VULKAN;
#endif

        return flags;
    }

    static int GetDisplayCount()
    {
        auto result = SDL_GetNumVideoDisplays();

        if (result < 0)
        {
            LOGF_ERROR("Failed to get display count: {}", SDL_GetError());
            return 1;
        }

        return result;
    }

    static int GetDisplay()
    {
        auto displayCount = GetDisplayCount();

        for (int i = 0; i < displayCount; i++)
        {
            SDL_Rect bounds;

            if (SDL_GetDisplayBounds(i, &bounds) == 0)
            {
                auto x = s_x;
                auto y = s_y;

                if (x == SDL_WINDOWPOS_CENTERED)
                    x = bounds.w / 2 - s_width / 2;

                if (y == SDL_WINDOWPOS_CENTERED)
                    y = bounds.h / 2 - s_height / 2;

                if (x >= bounds.x && x < bounds.x + bounds.w &&
                    y >= bounds.y && y < bounds.y + bounds.h)
                {
                    return i;
                }
            }
        }

        return 0;
    }

    static void SetDisplay(int displayIndex)
    {
        if (!IsFullscreen())
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

    static bool IsPositionValid()
    {
        auto displayCount = GetDisplayCount();

        for (int i = 0; i < displayCount; i++)
        {
            SDL_Rect bounds;

            if (SDL_GetDisplayBounds(i, &bounds) == 0)
            {
                auto x = s_x;
                auto y = s_y;

                if (!Config::Fullscreen && s_width == bounds.w && s_height == bounds.h)
                    return false;

                if (x == SDL_WINDOWPOS_CENTERED)
                    x = bounds.w / 2 - s_width / 2;

                if (y == SDL_WINDOWPOS_CENTERED)
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

    static void Init(bool sdlVideoDefault);
    static void Update();
};
