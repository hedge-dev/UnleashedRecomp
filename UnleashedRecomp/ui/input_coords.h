#pragma once

#include <imgui.h>
#include <SDL.h>

ImVec2 TransformWindowPointToViewport(float x, float y);
ImVec2 GetViewportPointFromSDLEvent(const SDL_Event* event);
bool IsPointInRect(const ImVec2& point, const ImVec2& min, const ImVec2& max);
