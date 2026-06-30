#include "touch_controls.h"

#include <stdafx.h>
#include <app.h>
#include <gpu/video.h>
#include <hid/hid.h>
#include <patches/aspect_ratio_patches.h>
#include <ui/game_window.h>
#include <ui/imgui_utils.h>
#include <ui/input_coords.h>
#include <ui/installer_wizard.h>
#include <user/config.h>
#include <sdl_listener.h>

#ifdef UNLEASHED_RECOMP_IOS

namespace
{
    struct VirtualButton
    {
        ImVec2 center{};
        float drawRadius{};
        float hitRadius{};
        uint16_t button{};
        SDL_FingerID fingerId{ -1 };
        bool pressed{};
    };

    struct VirtualStick
    {
        ImVec2 center{};
        float radius{};
        float hitRadius{};
        SDL_FingerID fingerId{ -1 };
        ImVec2 delta{};
    };

    struct ToggleButton
    {
        ImVec2 min{};
        ImVec2 max{};
        SDL_FingerID fingerId{ -1 };
    };

    static XAMINPUT_GAMEPAD g_gamepadState{};
    static VirtualStick g_leftStick{};
    static VirtualStick g_rightStick{};
    static std::vector<VirtualButton> g_buttons;
    static ToggleButton g_toggleButton{};

    static float ViewportHeight()
    {
        return Video::s_viewportHeight != 0 ? float(Video::s_viewportHeight) : float(GameWindow::s_height);
    }

    static float TouchScale()
    {
        return std::max(1.0f, ViewportHeight() / 720.0f);
    }

    static ImVec2 ScreenPoint(float x, float y)
    {
        return { std::round(x), std::round(y) };
    }

    static ImVec2 LogicalPoint(float x, float y)
    {
        return ScreenPoint(g_aspectRatioOffsetX + Scale(x), g_aspectRatioOffsetY + Scale(y));
    }

    static void ResetGamepadState()
    {
        g_gamepadState = {};
        g_leftStick.fingerId = -1;
        g_leftStick.delta = {};
        g_rightStick.fingerId = -1;
        g_rightStick.delta = {};

        for (auto& button : g_buttons)
        {
            button.fingerId = -1;
            button.pressed = false;
        }
    }

    static void SetButtonLayout(size_t index, const ImVec2& center, float drawRadius, uint16_t button)
    {
        if (index >= g_buttons.size())
            g_buttons.resize(index + 1);

        auto& virtualButton = g_buttons[index];
        virtualButton.center = center;
        virtualButton.drawRadius = drawRadius;
        virtualButton.hitRadius = std::max(drawRadius * 1.65f, 62.0f * TouchScale());
        virtualButton.button = button;
    }

    static void UpdateLayout()
    {
        const float scale = TouchScale();
        const float stickRadius = std::max(Scale(112.0f), 112.0f * scale);
        const float rightStickRadius = std::max(Scale(96.0f), 96.0f * scale);
        const float buttonRadius = std::max(Scale(58.0f), 58.0f * scale);
        const float smallButtonRadius = std::max(Scale(44.0f), 44.0f * scale);

        g_leftStick.center = LogicalPoint(106.0f, 590.0f);
        g_leftStick.radius = stickRadius;
        g_leftStick.hitRadius = stickRadius * 1.55f;

        g_rightStick.center = LogicalPoint(1118.0f, 594.0f);
        g_rightStick.radius = rightStickRadius;
        g_rightStick.hitRadius = rightStickRadius * 1.5f;

        SetButtonLayout(0, LogicalPoint(90.0f, 440.0f), smallButtonRadius, XAMINPUT_GAMEPAD_DPAD_UP);
        SetButtonLayout(1, LogicalPoint(90.0f, 540.0f), smallButtonRadius, XAMINPUT_GAMEPAD_DPAD_DOWN);
        SetButtonLayout(2, LogicalPoint(32.0f, 490.0f), smallButtonRadius, XAMINPUT_GAMEPAD_DPAD_LEFT);
        SetButtonLayout(3, LogicalPoint(148.0f, 490.0f), smallButtonRadius, XAMINPUT_GAMEPAD_DPAD_RIGHT);

        SetButtonLayout(4, LogicalPoint(74.0f, 86.0f), smallButtonRadius, XAMINPUT_GAMEPAD_LEFT_SHOULDER);
        SetButtonLayout(5, LogicalPoint(1200.0f, 86.0f), smallButtonRadius, XAMINPUT_GAMEPAD_RIGHT_SHOULDER);

        SetButtonLayout(6, LogicalPoint(1194.0f, 510.0f), buttonRadius, XAMINPUT_GAMEPAD_A);
        SetButtonLayout(7, LogicalPoint(1114.0f, 570.0f), buttonRadius, XAMINPUT_GAMEPAD_B);
        SetButtonLayout(8, LogicalPoint(1114.0f, 450.0f), buttonRadius, XAMINPUT_GAMEPAD_X);
        SetButtonLayout(9, LogicalPoint(1194.0f, 390.0f), buttonRadius, XAMINPUT_GAMEPAD_Y);

        SetButtonLayout(10, LogicalPoint(664.0f, 664.0f), smallButtonRadius, XAMINPUT_GAMEPAD_START);
        SetButtonLayout(11, LogicalPoint(564.0f, 664.0f), smallButtonRadius, XAMINPUT_GAMEPAD_BACK);

        g_toggleButton.min = LogicalPoint(1020.0f, 18.0f);
        g_toggleButton.max = LogicalPoint(1266.0f, 64.0f);
    }

    static bool IsTouchMouseEvent(const SDL_Event* event)
    {
        switch (event->type)
        {
            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP:
                return event->button.which == SDL_TOUCH_MOUSEID;

            case SDL_MOUSEMOTION:
                return event->motion.which == SDL_TOUCH_MOUSEID;

            default:
                return false;
        }
    }

    static bool IsPointInToggle(const ImVec2& point)
    {
        return point.x >= g_toggleButton.min.x && point.x <= g_toggleButton.max.x &&
            point.y >= g_toggleButton.min.y && point.y <= g_toggleButton.max.y;
    }

    static VirtualButton* FindButtonAtPoint(const ImVec2& point)
    {
        for (auto& button : g_buttons)
        {
            const ImVec2 delta = { point.x - button.center.x, point.y - button.center.y };
            const float distanceSq = delta.x * delta.x + delta.y * delta.y;
            if (distanceSq <= button.hitRadius * button.hitRadius)
                return &button;
        }

        return nullptr;
    }

    static VirtualStick* FindStickAtPoint(const ImVec2& point)
    {
        auto testStick = [&](VirtualStick& stick) -> VirtualStick*
        {
            const ImVec2 delta = { point.x - stick.center.x, point.y - stick.center.y };
            const float distanceSq = delta.x * delta.x + delta.y * delta.y;
            if (distanceSq <= stick.hitRadius * stick.hitRadius)
                return &stick;

            return nullptr;
        };

        if (auto* stick = testStick(g_leftStick))
            return stick;

        return testStick(g_rightStick);
    }

    static ImVec2 ClampDelta(const ImVec2& delta, float radius)
    {
        const float length = sqrtf(delta.x * delta.x + delta.y * delta.y);
        if (length <= radius || length <= 0.0f)
            return delta;

        const float scale = radius / length;
        return { delta.x * scale, delta.y * scale };
    }

    static void SetStickPoint(VirtualStick& stick, const ImVec2& point)
    {
        stick.delta = ClampDelta({ point.x - stick.center.x, point.y - stick.center.y }, stick.radius);
    }

    static void UpdateStickValue(VirtualStick& stick)
    {
        float normX = stick.delta.x / stick.radius;
        float normY = stick.delta.y / stick.radius;
        const float length = sqrtf(normX * normX + normY * normY);
        if (length > 1.0f)
        {
            normX /= length;
            normY /= length;
        }

        const int16_t axisX = int16_t(normX * 32767.0f);
        const int16_t axisY = int16_t(-normY * 32767.0f);

        if (&stick == &g_leftStick)
        {
            g_gamepadState.sThumbLX = axisX;
            g_gamepadState.sThumbLY = axisY;
        }
        else
        {
            g_gamepadState.sThumbRX = axisX;
            g_gamepadState.sThumbRY = axisY;
        }
    }

    static void ResetStick(VirtualStick& stick)
    {
        stick.fingerId = -1;
        stick.delta = {};

        if (&stick == &g_leftStick)
        {
            g_gamepadState.sThumbLX = 0;
            g_gamepadState.sThumbLY = 0;
        }
        else
        {
            g_gamepadState.sThumbRX = 0;
            g_gamepadState.sThumbRY = 0;
        }
    }

    static void RebuildGamepadButtons()
    {
        g_gamepadState.wButtons = 0;

        for (const auto& button : g_buttons)
        {
            if (button.pressed)
                g_gamepadState.wButtons |= button.button;
        }

        g_gamepadState.bLeftTrigger = (g_gamepadState.wButtons & XAMINPUT_GAMEPAD_LEFT_SHOULDER) ? 255 : 0;
        g_gamepadState.bRightTrigger = (g_gamepadState.wButtons & XAMINPUT_GAMEPAD_RIGHT_SHOULDER) ? 255 : 0;
    }

    static bool HandleTogglePress(const ImVec2& point, SDL_FingerID fingerId, bool pressed)
    {
        if (pressed)
        {
            if (!IsPointInToggle(point))
                return false;

            if (g_toggleButton.fingerId < 0)
                g_toggleButton.fingerId = fingerId;

            return true;
        }

        if (g_toggleButton.fingerId != fingerId)
            return false;

        if (IsPointInToggle(point))
        {
            Config::TouchControls = !Config::TouchControls;
            Config::Save();
            ResetGamepadState();
        }

        g_toggleButton.fingerId = -1;
        return true;
    }

    static bool HandlePointerDown(const ImVec2& point, SDL_FingerID fingerId)
    {
        if (HandleTogglePress(point, fingerId, true))
            return true;

        if (!TouchControls::IsEnabled())
            return false;

        if (VirtualButton* button = FindButtonAtPoint(point))
        {
            if (button->fingerId < 0)
            {
                button->fingerId = fingerId;
                button->pressed = true;
                RebuildGamepadButtons();
            }

            return true;
        }

        if (VirtualStick* stick = FindStickAtPoint(point))
        {
            if (stick->fingerId < 0)
            {
                stick->fingerId = fingerId;
                SetStickPoint(*stick, point);
                UpdateStickValue(*stick);
            }

            return true;
        }

        return false;
    }

    static bool HandlePointerMove(const ImVec2& point, SDL_FingerID fingerId)
    {
        if (!TouchControls::IsEnabled())
            return false;

        VirtualButton* ownedButton = nullptr;
        for (auto& button : g_buttons)
        {
            if (button.fingerId == fingerId)
            {
                ownedButton = &button;
                break;
            }
        }

        if (ownedButton)
        {
            VirtualButton* hoveredButton = FindButtonAtPoint(point);
            if (hoveredButton != ownedButton)
            {
                ownedButton->fingerId = -1;
                ownedButton->pressed = false;

                if (hoveredButton && hoveredButton->fingerId < 0)
                {
                    hoveredButton->fingerId = fingerId;
                    hoveredButton->pressed = true;
                }

                RebuildGamepadButtons();
            }

            return true;
        }

        for (VirtualStick* stick : { &g_leftStick, &g_rightStick })
        {
            if (stick->fingerId != fingerId)
                continue;

            SetStickPoint(*stick, point);
            UpdateStickValue(*stick);
            return true;
        }

        return false;
    }

    static bool HandlePointerUp(const ImVec2& point, SDL_FingerID fingerId)
    {
        if (g_toggleButton.fingerId == fingerId)
        {
            HandleTogglePress(point, fingerId, false);
            return true;
        }

        if (!TouchControls::IsEnabled())
            return false;

        for (VirtualStick* stick : { &g_leftStick, &g_rightStick })
        {
            if (stick->fingerId != fingerId)
                continue;

            ResetStick(*stick);
            return true;
        }

        bool releasedButton = false;
        for (auto& button : g_buttons)
        {
            if (button.fingerId == fingerId)
            {
                button.fingerId = -1;
                button.pressed = false;
                releasedButton = true;
            }
        }

        if (releasedButton)
        {
            RebuildGamepadButtons();
            return true;
        }

        return false;
    }

    static void DrawStick(const VirtualStick& stick, const char* label)
    {
        auto* drawList = ImGui::GetBackgroundDrawList();
        const ImU32 baseColor = IM_COL32(255, 255, 255, 55);
        const ImU32 activeColor = IM_COL32(120, 220, 255, 140);
        const ImU32 knobColor = IM_COL32(255, 255, 255, 200);

        drawList->AddCircle(stick.center, stick.radius, IM_COL32(255, 255, 255, 120), 32, Scale(3.0f));
        drawList->AddCircleFilled(stick.center, stick.radius, baseColor, 32);

        const ImVec2 knobCenter = { stick.center.x + stick.delta.x, stick.center.y + stick.delta.y };
        drawList->AddCircleFilled(knobCenter, stick.radius * 0.38f, stick.fingerId >= 0 ? activeColor : knobColor, 24);

        if (label && label[0] != '\0')
        {
            const auto* font = ImGui::GetFont();
            const float fontSize = Scale(14.0f);
            const ImVec2 textSize = font->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, label);
            drawList->AddText(font, fontSize, { stick.center.x - textSize.x * 0.5f, stick.center.y - stick.radius - Scale(20.0f) }, IM_COL32(255, 255, 255, 200), label);
        }
    }

    static void DrawButton(const VirtualButton& button, const char* label)
    {
        auto* drawList = ImGui::GetBackgroundDrawList();
        const ImU32 fillColor = button.pressed ? IM_COL32(120, 220, 255, 190) : IM_COL32(255, 255, 255, 70);
        const ImU32 borderColor = button.pressed ? IM_COL32(180, 240, 255, 230) : IM_COL32(255, 255, 255, 140);

        drawList->AddCircleFilled(button.center, button.drawRadius, fillColor, 24);
        drawList->AddCircle(button.center, button.drawRadius, borderColor, 24, Scale(2.5f));

        if (label && label[0] != '\0')
        {
            const auto* font = ImGui::GetFont();
            const float fontSize = Scale(16.0f);
            const ImVec2 textSize = font->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, label);
            drawList->AddText(
                font,
                fontSize,
                { button.center.x - textSize.x * 0.5f, button.center.y - textSize.y * 0.5f },
                IM_COL32(255, 255, 255, 240),
                label);
        }
    }

    static void DrawToggle()
    {
        auto* drawList = ImGui::GetBackgroundDrawList();
        const bool enabled = TouchControls::IsEnabled();
        const ImU32 fillColor = enabled ? IM_COL32(70, 150, 90, 170) : IM_COL32(90, 90, 90, 170);
        const char* label = enabled ? "Touch ON" : "Touch OFF";

        drawList->AddRectFilled(g_toggleButton.min, g_toggleButton.max, fillColor, Scale(8.0f));
        drawList->AddRect(g_toggleButton.min, g_toggleButton.max, IM_COL32(255, 255, 255, 180), Scale(8.0f), 0, Scale(2.0f));

        const auto* font = ImGui::GetFont();
        const float fontSize = Scale(16.0f);
        const ImVec2 textSize = font->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, label);
        const ImVec2 textPos = {
            (g_toggleButton.min.x + g_toggleButton.max.x - textSize.x) * 0.5f,
            (g_toggleButton.min.y + g_toggleButton.max.y - textSize.y) * 0.5f
        };
        drawList->AddText(font, fontSize, textPos, IM_COL32(255, 255, 255, 240), label);
    }

    class SDLEventListenerForTouchControls : public SDLEventListener
    {
    public:
        bool OnSDLEvent(SDL_Event* event) override
        {
            if (!TouchControls::IsActive())
                return false;

            UpdateLayout();

            switch (event->type)
            {
                case SDL_FINGERDOWN:
                {
                    const ImVec2 point = GetViewportPointFromSDLEvent(event);
                    return HandlePointerDown(point, event->tfinger.fingerId);
                }

                case SDL_FINGERMOTION:
                {
                    const ImVec2 point = GetViewportPointFromSDLEvent(event);
                    return HandlePointerMove(point, event->tfinger.fingerId);
                }

                case SDL_FINGERUP:
                {
                    const ImVec2 point = GetViewportPointFromSDLEvent(event);
                    return HandlePointerUp(point, event->tfinger.fingerId);
                }

                case SDL_MOUSEBUTTONDOWN:
                case SDL_MOUSEBUTTONUP:
                {
                    if (!IsTouchMouseEvent(event))
                        break;

                    const ImVec2 point = GetViewportPointFromSDLEvent(event);
                    const SDL_FingerID fingerId = 0;
                    if (event->type == SDL_MOUSEBUTTONDOWN)
                        return HandlePointerDown(point, fingerId);

                    return HandlePointerUp(point, fingerId);
                }

                case SDL_MOUSEMOTION:
                {
                    if (!IsTouchMouseEvent(event))
                        break;

                    const ImVec2 point = GetViewportPointFromSDLEvent(event);
                    return HandlePointerMove(point, 0);
                }
            }

            return false;
        }
    };

    static SDLEventListenerForTouchControls g_touchControlsListener;
}

void TouchControls::Init()
{
    UpdateLayout();
    ResetGamepadState();
}

bool TouchControls::IsActive()
{
    return App::s_isInit && !InstallerWizard::s_isVisible;
}

bool TouchControls::IsEnabled()
{
    return Config::TouchControls;
}

XAMINPUT_GAMEPAD TouchControls::GetState()
{
    return g_gamepadState;
}

void TouchControls::Draw()
{
    if (!IsActive())
        return;

    UpdateLayout();
    DrawToggle();

    if (!IsEnabled())
        return;

    DrawStick(g_leftStick, "Move");
    DrawStick(g_rightStick, "Camera");

    static const char* labels[] = { "^", "v", "<", ">", "LB", "RB", "A", "B", "X", "Y", "Start", "Back" };
    for (size_t i = 0; i < g_buttons.size() && i < std::size(labels); i++)
        DrawButton(g_buttons[i], labels[i]);
}

#else

void TouchControls::Init() {}
bool TouchControls::IsActive() { return false; }
bool TouchControls::IsEnabled() { return false; }
XAMINPUT_GAMEPAD TouchControls::GetState() { return {}; }
void TouchControls::Draw() {}

#endif
