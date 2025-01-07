#include "reddog_manager.h"
#include <gpu/video.h>
#include <ui/reddog/windows/window_list.h>
#include <ui/reddog/reddog_controls.h>
#include <ui/game_window.h>
#include <ui/imgui_utils.h>

#include <res/images/reddog/debug_icon.dds.h>

static std::unique_ptr<GuestTexture> g_upDebugIcon;

static ImVec2 g_debugIconPosOrig;
static ImVec2 g_debugIconPos = { 0, 0 };

static bool g_isDebugIconClicked = false;
static bool g_isDraggingDebugIcon = false;
static bool g_isReddogToggled = false;
static bool g_isWindowListVisible = false;

void Reddog::Manager::Init()
{
    g_upDebugIcon = LoadTexture(g_debug_icon, sizeof(g_debug_icon));

    Reddog::InitControlsResources();
    Reddog::InitWindowResources();
}

void Reddog::Manager::Draw()
{
    bool isReddogToggled = SDL_GetKeyboardState(nullptr)[SDL_SCANCODE_F1] != 0;

    if (!g_isReddogToggled && isReddogToggled)
    {
        s_isVisible = !s_isVisible;

        if (!s_isVisible)
        {
            g_debugIconPos = { 0, 0 };
            GameWindow::SetCursor(ECursorType::Default);
            GameWindow::SetCursorVisibility(false);
        }
    }

    g_isReddogToggled = isReddogToggled;

    if (!s_isVisible)
        return;

    if (GameWindow::s_currentCursor == ECursorType::Default)
    {
        GameWindow::SetCursor(ECursorType::DebugDefault);
        GameWindow::SetCursorVisibility(true);
    }

    auto drawList = ImGui::GetForegroundDrawList();
    auto& res = ImGui::GetIO().DisplaySize;

    auto debugIconSize = Scale(32);
    ImVec2 debugIconMin = g_debugIconPos;
    ImVec2 debugIconMax = { g_debugIconPos.x + debugIconSize, g_debugIconPos.y + debugIconSize };

    drawList->AddImage(g_upDebugIcon.get(), debugIconMin, debugIconMax);

    if (ImGui::IsMouseHoveringRect(debugIconMin, debugIconMax, false) && ImGui::IsMouseClicked(0))
    {
        g_debugIconPosOrig = g_debugIconPos;
        g_isDraggingDebugIcon = true;
    }

    g_isDebugIconClicked = false;

    if (g_isDraggingDebugIcon && ImGui::IsMouseDown(0))
    {
        auto& delta = ImGui::GetIO().MouseDelta;
        g_debugIconPos.x += delta.x;
        g_debugIconPos.y += delta.y;
    }
    else
    {
        if (g_isDraggingDebugIcon &&
            g_debugIconPos.x == g_debugIconPosOrig.x &&
            g_debugIconPos.y == g_debugIconPosOrig.y)
        {
            g_isDebugIconClicked = true;
            g_isWindowListVisible = !g_isWindowListVisible;
        }

        g_isDraggingDebugIcon = false;
    }

    if (auto pWindowList = GetWindow<WindowList>(WINDOW_LIST_NAME))
    {
        if (!g_isDebugIconClicked && !pWindowList->IsFocused)
            pWindowList->IsVisible = g_isWindowListVisible = false;

        pWindowList->IsVisible = g_isWindowListVisible;

        if (pWindowList->ImWindow)
        {
            if (debugIconMin.x > res.x / 2)
            {
                pWindowList->ImWindow->Pos.x = debugIconMax.x - pWindowList->ImWindow->Size.x;
            }
            else
            {
                pWindowList->ImWindow->Pos.x = debugIconMin.x;
            }

            if (debugIconMin.y > res.y / 2)
            {
                pWindowList->ImWindow->Pos.y = debugIconMin.y - pWindowList->ImWindow->Size.y;
            }
            else
            {
                pWindowList->ImWindow->Pos.y = debugIconMin.y + debugIconSize;
            }
        }
    }

    for (auto& pWindow : Reddog::GetWindows())
    {
        auto pTrueWindow = (Reddog::Window*)pWindow;

        if (!pTrueWindow->IsVisible)
            continue;

        pWindow->Draw();
    }
}

Reddog::Window* Reddog::Manager::GetWindow(const char* pName)
{
    for (auto pWindow : Reddog::GetWindows())
    {
        if (strcmp(pWindow->GetName(), pName) == 0)
            return (Reddog::Window*)pWindow;
    }

    return nullptr;
}

template<typename T>
T* Reddog::Manager::GetWindow(const char* pName)
{
    return (T*)Reddog::Manager::GetWindow(pName);
}
