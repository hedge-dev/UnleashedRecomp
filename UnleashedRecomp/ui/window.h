#pragma once

#include <res/images/game_icon.bmp.h>
#include <res/images/game_icon_night.bmp.h>
#include <ui/window_events.h>
#include <user/config.h>

#if _WIN32
#include <dwmapi.h>
#include <kernel/platform.h>
#pragma comment(lib, "dwmapi.lib")
#endif

#define DEFAULT_WIDTH 1280
#define DEFAULT_HEIGHT 720

class Window
{
public:
    inline static SDL_Window* s_pWindow;
    inline static HWND s_handle;

    inline static int s_x;
    inline static int s_y;
    inline static int s_width = DEFAULT_WIDTH;
    inline static int s_height = DEFAULT_HEIGHT;

    inline static bool s_isFocused;
    inline static bool s_isIconNight;
    inline static bool s_cursorAllowed = false;

    static SDL_Surface* GetIconSurface(void* pIconBmp, size_t iconSize)
    {
        auto rw = SDL_RWFromMem(pIconBmp, iconSize);
        auto surface = SDL_LoadBMP_RW(rw, 1);

        if (!surface)
            printf("Failed to load icon: %s\n", SDL_GetError());

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
        auto version = GetPlatformVersion();

        if (version.Major < 10 || version.Build <= 17763)
            return;

        auto flag = version.Build >= 18985
            ? DWMWA_USE_IMMERSIVE_DARK_MODE
            : 19; // DWMWA_USE_IMMERSIVE_DARK_MODE_BEFORE_20H1

        const DWORD useImmersiveDarkMode = isEnabled;
        DwmSetWindowAttribute(s_handle, flag, &useImmersiveDarkMode, sizeof(useImmersiveDarkMode));
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
            SDL_ShowCursor(s_cursorAllowed ? SDL_ENABLE : SDL_DISABLE);
        }
        else
        {
            SDL_SetWindowFullscreen(s_pWindow, 0);
            SDL_ShowCursor(SDL_ENABLE);
            SetIcon(Window::s_isIconNight);
        }

        return isEnabled;
    }
    
    static void SetCursorAllowed(bool isCursorAllowed)
    {
        s_cursorAllowed = isCursorAllowed;

        // Refresh fullscreen state to enable the right cursor behavior.
        SetFullscreen(IsFullscreen());
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

    static uint32_t GetWindowFlags()
    {
        uint32_t flags = SDL_WINDOW_HIDDEN | SDL_WINDOW_RESIZABLE;

        if (Config::WindowState == EWindowState::Maximised)
            flags |= SDL_WINDOW_MAXIMIZED;

        if (Config::Fullscreen)
            flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;

        return flags;
    }

    static bool IsPositionValid()
    {
        auto displayCount = SDL_GetNumVideoDisplays();

        if (displayCount <= 0)
        {
            printf("Failed to validate window position: %s\n", SDL_GetError());
            return false;
        }

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

    static void Init();
    static void Update();
};
