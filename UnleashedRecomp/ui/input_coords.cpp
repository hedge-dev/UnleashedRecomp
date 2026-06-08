#include "input_coords.h"

#include <gpu/video.h>
#include <ui/game_window.h>

static ImVec2 GetCurrentWindowSize()
{
    int width = GameWindow::s_width;
    int height = GameWindow::s_height;

    if (GameWindow::s_pWindow)
        SDL_GetWindowSize(GameWindow::s_pWindow, &width, &height);

    return { float(width), float(height) };
}

ImVec2 TransformWindowPointToViewport(float x, float y)
{
    const ImVec2 windowSize = GetCurrentWindowSize();

    if (windowSize.x <= 0.0f || windowSize.y <= 0.0f || Video::s_drawableWidth == 0 || Video::s_drawableHeight == 0)
        return { x, y };

    const float scaleX = float(Video::s_drawableWidth) / windowSize.x;
    const float scaleY = float(Video::s_drawableHeight) / windowSize.y;
    const float offsetX = (Video::s_drawableWidth - Video::s_viewportWidth) / 2.0f;
    const float offsetY = (Video::s_drawableHeight - Video::s_viewportHeight) / 2.0f;

    return { x * scaleX - offsetX, y * scaleY - offsetY };
}

ImVec2 GetViewportPointFromSDLEvent(const SDL_Event* event)
{
    switch (event->type)
    {
        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP:
            return TransformWindowPointToViewport(float(event->button.x), float(event->button.y));

        case SDL_MOUSEMOTION:
            return TransformWindowPointToViewport(float(event->motion.x), float(event->motion.y));

        case SDL_FINGERDOWN:
        case SDL_FINGERUP:
        case SDL_FINGERMOTION:
        {
            const ImVec2 windowSize = GetCurrentWindowSize();
            return TransformWindowPointToViewport(
                event->tfinger.x * windowSize.x,
                event->tfinger.y * windowSize.y);
        }

        default:
            return {};
    }
}

bool IsPointInRect(const ImVec2& point, const ImVec2& min, const ImVec2& max)
{
    return point.x >= min.x && point.x <= max.x && point.y >= min.y && point.y <= max.y;
}
