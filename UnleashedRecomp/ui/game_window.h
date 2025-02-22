#pragma once

#include <gpu/rhi/plume_render_interface_types.h>
#include <user/config.h>
#include <sdl_events.h>

#define DEFAULT_WIDTH 1280
#define DEFAULT_HEIGHT 720
#define MIN_WIDTH 640
#define MIN_HEIGHT 480

class GameWindow
{
public:
    static inline SDL_Window* s_pWindow = nullptr;
    static inline plume::RenderWindow s_renderWindow;

    static inline int s_x;
    static inline int s_y;
    static inline int s_width = DEFAULT_WIDTH;
    static inline int s_height = DEFAULT_HEIGHT;

    static inline bool s_isFocused;
    static inline bool s_isIconNight;
    static inline bool s_isFullscreenCursorVisible;
    static inline bool s_isChangingDisplay;

    static SDL_Surface* GetIconSurface(void* pIconBmp, size_t iconSize);
    static void SetIcon(void* pIconBmp, size_t iconSize);
    static void SetIcon(bool isNight = false);
    static const char* GetTitle();
    static void SetTitle(const char* title = nullptr);
    static void SetTitleBarColour();
    static bool IsFullscreen();
    static bool SetFullscreen(bool isEnabled);
    static void SetFullscreenCursorVisibility(bool isVisible);
    static bool IsMaximised();
    static EWindowState SetMaximised(bool isEnabled);
    static SDL_Rect GetDimensions();
    static void GetSizeInPixels(int *w, int *h);
    static void SetDimensions(int w, int h, int x = SDL_WINDOWPOS_CENTERED, int y = SDL_WINDOWPOS_CENTERED);
    static void ResetDimensions();
    static uint32_t GetWindowFlags();
    static int GetDisplayCount();
    static int GetDisplay();
    static void SetDisplay(int displayIndex);
    static std::vector<SDL_DisplayMode> GetDisplayModes(bool ignoreInvalidModes = true, bool ignoreRefreshRates = true);
    static int FindNearestDisplayMode();
    static bool IsPositionValid();
    static void Init(const char* sdlVideoDriver = nullptr);
    static void Update();
};
