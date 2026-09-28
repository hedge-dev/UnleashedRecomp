#include "touch_controls.h"
#include "installer_wizard.h"
#include "imgui_utils.h"
#include <gpu/imgui/imgui_snapshot.h>
#include <hid/hid.h>
#include <os/logger.h>
#include <user/config.h>
#include <user/paths.h>
#include <app.h>
#include <ui/game_window.h>

#if defined(__APPLE__)
#include <TargetConditionals.h>
#endif

#if defined(TARGET_OS_IPHONE) && TARGET_OS_IPHONE
#include "touch_haptics.h"
#define TOUCH_CONTROLS_SUPPORTED 1
#else
#define TOUCH_CONTROLS_SUPPORTED 0
#endif

// Layout frames and tuning values match the XeniOS defaults (https://github.com/xenios-jp/XeniOS/src/xenia/hid/touch).
static constexpr float STICK_DEADZONE = 0.14f;
static constexpr float STICK_ACTIVATION_RADIUS = 0.48f;
static constexpr float STICK_DPAD_RING_RADIUS = 0.32f;
static constexpr float LOOK_POINTS_PER_FULL_SCALE = 4.0f;
static constexpr float LOOK_VERTICAL_SCALE = 1.2f;
static constexpr double LOOK_HOLD_SECONDS = 0.10;
static constexpr double BUTTON_TAP_HOLD_SECONDS = 0.10;

// Horizontal margin kept free on wide screens so controls stay clear of the notch and rounded corners.
static constexpr float WIDE_SCREEN_SAFE_MARGIN = 0.055f;

// Layout editor limits, in layout space.
static constexpr float EDIT_MIN_SIZE = 0.04f;
static constexpr float EDIT_MAX_SIZE = 0.45f;
static constexpr float EDIT_SIZE_STEP = 1.1f;
static constexpr float EDIT_OPACITY_STEP = 0.1f;
static constexpr float EDIT_MIN_OPACITY = 0.1f;

static constexpr const char* LAYOUT_FILE_NAME = "touch_layout.toml";

struct TouchRect
{
    float x, y, width, height;
};

enum class TouchControlKind
{
    Stick,
    Button
};

struct TouchControl
{
    TouchControlKind kind;
    const char* id;
    const char* label;
    TouchRect frame; // Default frame, normalised to the layout space.
    uint16_t buttons;
    uint8_t leftTrigger;
    uint8_t rightTrigger;
};

// Default frames come from the XeniOS default layout, with its actions mapped to their gamepad equivalents.
static const TouchControl g_controls[] =
{
    { TouchControlKind::Stick,  "stick", nullptr, { 0.055f, 0.560f, 0.190f, 0.315f }, 0, 0, 0 },
    { TouchControlKind::Button, "back",  "BACK",  { 0.390f, 0.045f, 0.080f, 0.112f }, XAMINPUT_GAMEPAD_BACK, 0, 0 },
    { TouchControlKind::Button, "start", "START", { 0.495f, 0.045f, 0.085f, 0.112f }, XAMINPUT_GAMEPAD_START, 0, 0 },
    { TouchControlKind::Button, "lb",    "LB",    { 0.660f, 0.050f, 0.085f, 0.112f }, XAMINPUT_GAMEPAD_LEFT_SHOULDER, 0, 0 },
    { TouchControlKind::Button, "rb",    "RB",    { 0.765f, 0.050f, 0.085f, 0.112f }, XAMINPUT_GAMEPAD_RIGHT_SHOULDER, 0, 0 },
    { TouchControlKind::Button, "lt",    "LT",    { 0.095f, 0.405f, 0.120f, 0.110f }, 0, 0xFF, 0 },
    { TouchControlKind::Button, "rt",    "RT",    { 0.860f, 0.405f, 0.120f, 0.110f }, 0, 0, 0xFF },
    { TouchControlKind::Button, "y",     "Y",     { 0.760f, 0.455f, 0.065f, 0.115f }, XAMINPUT_GAMEPAD_Y, 0, 0 },
    { TouchControlKind::Button, "x",     "X",     { 0.700f, 0.585f, 0.065f, 0.115f }, XAMINPUT_GAMEPAD_X, 0, 0 },
    { TouchControlKind::Button, "b",     "B",     { 0.820f, 0.585f, 0.065f, 0.115f }, XAMINPUT_GAMEPAD_B, 0, 0 },
    { TouchControlKind::Button, "a",     "A",     { 0.760f, 0.715f, 0.065f, 0.115f }, XAMINPUT_GAMEPAD_A, 0, 0 },
};

static constexpr size_t CONTROL_COUNT = std::size(g_controls);

// The button that opens the layout editor. It isn't part of the editable layout.
static constexpr TouchRect EDIT_BUTTON_FRAME = { 0.290f, 0.045f, 0.075f, 0.112f };

struct LayoutEntry
{
    TouchRect frame;
    bool hidden;
};

static LayoutEntry g_layout[CONTROL_COUNT];

enum class EditorAction
{
    Smaller,
    Bigger,
    ToggleHidden,
    LessOpaque,
    MoreOpaque,
    Reset,
    Done
};

struct EditorButton
{
    EditorAction action;
    const char* label;
};

static const EditorButton g_editorButtons[] =
{
    { EditorAction::Smaller,      "SIZE -" },
    { EditorAction::Bigger,       "SIZE +" },
    { EditorAction::ToggleHidden, "HIDE" },
    { EditorAction::LessOpaque,   "ALPHA -" },
    { EditorAction::MoreOpaque,   "ALPHA +" },
    { EditorAction::Reset,        "RESET" },
    { EditorAction::Done,         "DONE" },
};

static constexpr size_t EDITOR_BUTTON_COUNT = std::size(g_editorButtons);

// Special capture targets besides the layout's controls.
static constexpr int LOOK_ZONE = -1;
static constexpr int EDIT_BUTTON = -2;
static constexpr int EDITOR_TOOLBAR = -3;
static constexpr int EDITOR_EMPTY = -4;

enum class StickZone
{
    Stick,
    DpadUp,
    DpadDown,
    DpadLeft,
    DpadRight
};

struct TouchCapture
{
    SDL_FingerID fingerId;
    int control;
    StickZone stickZone;
    ImVec2 anchor;  // Window points.
    ImVec2 current; // Window points.
    TouchRect frameAtAnchor; // Layout space, used when dragging in the editor.
};

static std::mutex g_mutex;
static std::vector<TouchCapture> g_captures;
static double g_buttonPressTimes[CONTROL_COUNT];
static ImVec2 g_lookVector;
static double g_lookTime = -1.0;
static ImVec2 g_windowSize{ 1.0f, 1.0f };
static std::atomic<bool> g_hiddenByController;
static std::atomic<bool> g_isEditing;
static int g_selectedControl = -1;
static ImFont* g_font;

static double GetTime()
{
    return double(SDL_GetPerformanceCounter()) / double(SDL_GetPerformanceFrequency());
}

static bool IsEnabled()
{
#if TOUCH_CONTROLS_SUPPORTED
    return Config::TouchControls && !InstallerWizard::s_isVisible;
#else
    return false;
#endif
}

static std::filesystem::path GetLayoutPath()
{
    return GetUserPath() / LAYOUT_FILE_NAME;
}

static void ResetLayout()
{
    for (size_t i = 0; i < CONTROL_COUNT; i++)
        g_layout[i] = { g_controls[i].frame, false };
}

static void LoadLayout()
{
    ResetLayout();

    std::error_code ec;
    if (!std::filesystem::exists(GetLayoutPath(), ec))
        return;

    try
    {
        std::ifstream stream(GetLayoutPath());
        toml::table table = toml::parse(stream);

        for (size_t i = 0; i < CONTROL_COUNT; i++)
        {
            auto entry = table[g_controls[i].id];
            if (!entry.is_table())
                continue;

            TouchRect& frame = g_layout[i].frame;
            frame.x = std::clamp(entry["x"].value_or(frame.x), 0.0f, 1.0f);
            frame.y = std::clamp(entry["y"].value_or(frame.y), 0.0f, 1.0f);
            frame.width = std::clamp(entry["width"].value_or(frame.width), EDIT_MIN_SIZE, EDIT_MAX_SIZE);
            frame.height = std::clamp(entry["height"].value_or(frame.height), EDIT_MIN_SIZE, EDIT_MAX_SIZE);
            g_layout[i].hidden = entry["hidden"].value_or(false);
        }
    }
    catch (toml::parse_error& err)
    {
        LOGFN_ERROR("Failed to parse touch layout: {}", err.what());
        ResetLayout();
    }
}

static void SaveLayout()
{
    toml::table table;

    for (size_t i = 0; i < CONTROL_COUNT; i++)
    {
        const LayoutEntry& entry = g_layout[i];

        table.insert(g_controls[i].id, toml::table
        {
            { "x", entry.frame.x },
            { "y", entry.frame.y },
            { "width", entry.frame.width },
            { "height", entry.frame.height },
            { "hidden", entry.hidden }
        });
    }

    std::ofstream stream(GetLayoutPath());
    if (stream.is_open())
        stream << table;
    else
        LOGN_ERROR("Failed to save touch layout.");
}

static TouchRect GetLayoutSpace(ImVec2 size)
{
    float margin = (size.x / size.y) > 1.9f ? size.x * WIDE_SCREEN_SAFE_MARGIN : 0.0f;
    return { margin, 0.0f, size.x - margin * 2.0f, size.y };
}

// Resolves a frame in layout space to window points.
static TouchRect ToWindowFrame(const TouchRect& frame, ImVec2 size)
{
    TouchRect space = GetLayoutSpace(size);

    return
    {
        space.x + frame.x * space.width,
        space.y + frame.y * space.height,
        frame.width * space.width,
        frame.height * space.height
    };
}

static TouchRect GetControlFrame(size_t index)
{
    return ToWindowFrame(g_layout[index].frame, g_windowSize);
}

// The editor toolbar runs along the bottom of the screen, in window points.
static TouchRect GetEditorButtonFrame(size_t index)
{
    TouchRect space = GetLayoutSpace(g_windowSize);
    float spacing = space.width * 0.01f;
    float width = (space.width * 0.9f - spacing * float(EDITOR_BUTTON_COUNT - 1)) / float(EDITOR_BUTTON_COUNT);
    float height = g_windowSize.y * 0.11f;
    float startX = space.x + space.width * 0.05f;

    return { startX + float(index) * (width + spacing), g_windowSize.y * 0.86f, width, height };
}

static ImVec2 GetCentre(const TouchRect& rect)
{
    return { rect.x + rect.width * 0.5f, rect.y + rect.height * 0.5f };
}

static float GetRadius(const TouchRect& rect)
{
    return std::min(rect.width, rect.height) * 0.5f;
}

static bool RectContainsPoint(const TouchRect& rect, ImVec2 point)
{
    return point.x >= rect.x && point.x <= rect.x + rect.width &&
        point.y >= rect.y && point.y <= rect.y + rect.height;
}

// Buttons are pills: circles, stretched along their longer side.
static bool PillContainsPoint(const TouchRect& frame, ImVec2 point)
{
    float radius = GetRadius(frame);
    ImVec2 centre = GetCentre(frame);
    float halfSpanX = std::max(frame.width * 0.5f - radius, 0.0f);
    float halfSpanY = std::max(frame.height * 0.5f - radius, 0.0f);
    float dx = std::max(std::abs(point.x - centre.x) - halfSpanX, 0.0f);
    float dy = std::max(std::abs(point.y - centre.y) - halfSpanY, 0.0f);
    return dx * dx + dy * dy <= radius * radius;
}

static bool ControlContainsPoint(size_t index, ImVec2 point)
{
    TouchRect frame = GetControlFrame(index);

    if (g_controls[index].kind == TouchControlKind::Stick)
        return RectContainsPoint(frame, point);

    return PillContainsPoint(frame, point);
}

static StickZone GetStickZone(const TouchRect& frame, ImVec2 point)
{
    ImVec2 centre = GetCentre(frame);
    float stickRadius = std::min(frame.width, frame.height) * STICK_DPAD_RING_RADIUS;
    float dx = point.x - centre.x;
    float dy = point.y - centre.y;

    if (dx * dx + dy * dy <= stickRadius * stickRadius)
        return StickZone::Stick;

    if (std::abs(dx) > std::abs(dy))
        return dx > 0.0f ? StickZone::DpadRight : StickZone::DpadLeft;

    return dy > 0.0f ? StickZone::DpadDown : StickZone::DpadUp;
}

// Returns the stick deflection for a capture, with Y pointing down.
static ImVec2 GetStickVector(const TouchRect& frame, const TouchCapture& capture)
{
    float outerRadius = std::min(frame.width, frame.height) * STICK_ACTIVATION_RADIUS;
    ImVec2 delta = { capture.current.x - capture.anchor.x, capture.current.y - capture.anchor.y };
    float distance = std::hypot(delta.x, delta.y);

    if (distance > outerRadius && distance > 0.0f)
    {
        delta.x *= outerRadius / distance;
        delta.y *= outerRadius / distance;
    }

    ImVec2 normalised = { delta.x / outerRadius, delta.y / outerRadius };
    float magnitude = std::hypot(normalised.x, normalised.y);

    if (magnitude < STICK_DEADZONE || magnitude <= 0.0f)
        return {};

    float rescaled = std::clamp((magnitude - STICK_DEADZONE) / (1.0f - STICK_DEADZONE), 0.0f, 1.0f);
    return { normalised.x / magnitude * rescaled, normalised.y / magnitude * rescaled };
}

static int16_t ToAxis(float value)
{
    return int16_t(std::lround(std::clamp(value, -1.0f, 1.0f) * 32767.0f));
}

static void MergeAxis(int16_t& target, int16_t value)
{
    if (std::abs(int(value)) > std::abs(int(target)))
        target = value;
}

static int FindControlAtPoint(ImVec2 point, bool includeHidden)
{
    // Buttons take priority over the stick.
    for (int pass = 0; pass < 2; pass++)
    {
        TouchControlKind kind = pass == 0 ? TouchControlKind::Button : TouchControlKind::Stick;

        for (size_t i = 0; i < CONTROL_COUNT; i++)
        {
            if (g_controls[i].kind == kind && (includeHidden || !g_layout[i].hidden) && ControlContainsPoint(i, point))
                return int(i);
        }
    }

    return -1;
}

static void PlayHaptic(bool light)
{
#if TOUCH_CONTROLS_SUPPORTED
    TouchHaptics::Play(light);
#endif
}

static void ClampToLayout(TouchRect& frame)
{
    frame.width = std::clamp(frame.width, EDIT_MIN_SIZE, EDIT_MAX_SIZE);
    frame.height = std::clamp(frame.height, EDIT_MIN_SIZE, EDIT_MAX_SIZE);
    frame.x = std::clamp(frame.x, 0.0f, 1.0f - frame.width);
    frame.y = std::clamp(frame.y, 0.0f, 1.0f - frame.height);
}

static void ApplyEditorAction(EditorAction action)
{
    switch (action)
    {
        case EditorAction::Smaller:
        case EditorAction::Bigger:
        {
            if (g_selectedControl < 0)
                break;

            TouchRect& frame = g_layout[g_selectedControl].frame;
            ImVec2 centre = GetCentre(frame);
            float scale = action == EditorAction::Bigger ? EDIT_SIZE_STEP : 1.0f / EDIT_SIZE_STEP;
            float newWidth = frame.width * scale;
            float newHeight = frame.height * scale;

            // Stop at the limits instead of clamping one side, so the control keeps its shape.
            if (newWidth < EDIT_MIN_SIZE || newHeight < EDIT_MIN_SIZE || newWidth > EDIT_MAX_SIZE || newHeight > EDIT_MAX_SIZE)
                break;

            frame = { centre.x - newWidth * 0.5f, centre.y - newHeight * 0.5f, newWidth, newHeight };
            ClampToLayout(frame);
            break;
        }

        case EditorAction::ToggleHidden:
        {
            if (g_selectedControl >= 0)
                g_layout[g_selectedControl].hidden = !g_layout[g_selectedControl].hidden;

            break;
        }

        case EditorAction::LessOpaque:
        case EditorAction::MoreOpaque:
        {
            float step = action == EditorAction::MoreOpaque ? EDIT_OPACITY_STEP : -EDIT_OPACITY_STEP;
            Config::TouchControlsOpacity = std::clamp(Config::TouchControlsOpacity + step, EDIT_MIN_OPACITY, 1.0f);
            break;
        }

        case EditorAction::Reset:
        {
            ResetLayout();
            Config::TouchControlsOpacity = 1.0f;
            break;
        }

        case EditorAction::Done:
        {
            SaveLayout();
            Config::Save();
            g_isEditing = false;
            g_selectedControl = -1;
            g_captures.clear();
            break;
        }
    }
}

static void OnEditorFingerDown(TouchCapture& capture, ImVec2 point)
{
    for (size_t i = 0; i < EDITOR_BUTTON_COUNT; i++)
    {
        if (RectContainsPoint(GetEditorButtonFrame(i), point))
        {
            capture.control = EDITOR_TOOLBAR;
            PlayHaptic(false);
            ApplyEditorAction(g_editorButtons[i].action);
            return;
        }
    }

    int control = FindControlAtPoint(point, true);
    if (control >= 0)
    {
        capture.control = control;
        capture.frameAtAnchor = g_layout[control].frame;
        g_selectedControl = control;
        PlayHaptic(true);
    }
    else
    {
        capture.control = EDITOR_EMPTY;
        g_selectedControl = -1;
    }
}

static void OnEditorFingerMotion(TouchCapture& capture)
{
    if (capture.control < 0)
        return;

    TouchRect space = GetLayoutSpace(g_windowSize);
    TouchRect& frame = g_layout[capture.control].frame;

    frame.x = capture.frameAtAnchor.x + (capture.current.x - capture.anchor.x) / space.width;
    frame.y = capture.frameAtAnchor.y + (capture.current.y - capture.anchor.y) / space.height;
    ClampToLayout(frame);
}

static void OnGameplayFingerDown(TouchCapture& capture, ImVec2 point)
{
    if (RectContainsPoint(ToWindowFrame(EDIT_BUTTON_FRAME, g_windowSize), point))
    {
        capture.control = EDIT_BUTTON;
        g_isEditing = true;
        g_selectedControl = -1;
        PlayHaptic(false);
        return;
    }

    capture.control = FindControlAtPoint(point, false);

    if (capture.control < 0)
    {
        capture.control = LOOK_ZONE;
        return;
    }

    const TouchControl& control = g_controls[capture.control];

    if (control.kind == TouchControlKind::Stick)
    {
        capture.stickZone = GetStickZone(GetControlFrame(capture.control), point);
        PlayHaptic(true);
    }
    else
    {
        g_buttonPressTimes[capture.control] = GetTime();
        PlayHaptic(false);
    }
}

static int TouchControls_OnSDLEvent(void*, SDL_Event* event)
{
    if (event->type != SDL_FINGERDOWN && event->type != SDL_FINGERMOTION && event->type != SDL_FINGERUP)
        return 0;

    std::lock_guard lock(g_mutex);

    if (!IsEnabled())
    {
        g_captures.clear();
        return 0;
    }

    int windowWidth = 0;
    int windowHeight = 0;
    SDL_GetWindowSize(GameWindow::s_pWindow, &windowWidth, &windowHeight);

    if (windowWidth > 0 && windowHeight > 0)
        g_windowSize = { float(windowWidth), float(windowHeight) };

    ImVec2 point = { event->tfinger.x * g_windowSize.x, event->tfinger.y * g_windowSize.y };
    auto capture = std::find_if(g_captures.begin(), g_captures.end(), [&](const TouchCapture& c) { return c.fingerId == event->tfinger.fingerId; });

    switch (event->type)
    {
        case SDL_FINGERDOWN:
        {
            g_hiddenByController = false;

            if (!App::s_isLoading)
            {
                // Show Xbox button prompts, matching the on-screen labels.
                hid::g_inputDevice = hid::EInputDevice::Xbox;
                hid::g_inputDeviceController = hid::EInputDevice::Xbox;
            }

            if (capture != g_captures.end())
                g_captures.erase(capture);

            TouchCapture newCapture{};
            newCapture.fingerId = event->tfinger.fingerId;
            newCapture.anchor = point;
            newCapture.current = point;

            if (g_isEditing)
                OnEditorFingerDown(newCapture, point);
            else
                OnGameplayFingerDown(newCapture, point);

            // Pressing DONE clears the captures, so don't track that touch any further.
            if (g_isEditing || newCapture.control != EDITOR_TOOLBAR)
                g_captures.push_back(newCapture);

            break;
        }

        case SDL_FINGERMOTION:
        {
            if (capture == g_captures.end())
                break;

            capture->current = point;

            if (g_isEditing)
            {
                OnEditorFingerMotion(*capture);
            }
            else if (capture->control == LOOK_ZONE)
            {
                float deltaX = event->tfinger.dx * g_windowSize.x;
                float deltaY = event->tfinger.dy * g_windowSize.y;

                g_lookVector =
                {
                    std::clamp(deltaX / LOOK_POINTS_PER_FULL_SCALE, -1.0f, 1.0f),
                    std::clamp(-deltaY / LOOK_POINTS_PER_FULL_SCALE * LOOK_VERTICAL_SCALE, -1.0f, 1.0f)
                };

                g_lookTime = GetTime();
            }

            break;
        }

        case SDL_FINGERUP:
        {
            if (capture != g_captures.end())
                g_captures.erase(capture);

            break;
        }
    }

    return 0;
}

void TouchControls::Init()
{
    g_font = ImFontAtlasSnapshot::GetFont("FOT-NewRodinPro-M.otf");

#if TOUCH_CONTROLS_SUPPORTED
    {
        std::lock_guard lock(g_mutex);
        LoadLayout();
    }

    TouchHaptics::Init();
    SDL_AddEventWatch(TouchControls_OnSDLEvent, nullptr);
#endif
}

bool TouchControls::IsActive()
{
    return IsEnabled() && !g_hiddenByController;
}

void TouchControls::OnPhysicalControllerInput()
{
    std::lock_guard lock(g_mutex);

    // Keep the overlay up while its layout is being edited.
    if (g_isEditing)
        return;

    g_hiddenByController = true;
    g_captures.clear();
    g_lookTime = -1.0;
}

void TouchControls::Apply(XAMINPUT_GAMEPAD& pad)
{
    // No game input while the layout is being edited.
    if (!IsActive() || g_isEditing)
        return;

    std::lock_guard lock(g_mutex);

    double time = GetTime();
    uint16_t buttons = 0;
    uint8_t leftTrigger = 0;
    uint8_t rightTrigger = 0;
    int16_t leftX = 0;
    int16_t leftY = 0;
    int16_t rightX = 0;
    int16_t rightY = 0;

    for (auto& capture : g_captures)
    {
        if (capture.control < 0)
            continue;

        const TouchControl& control = g_controls[capture.control];

        if (control.kind == TouchControlKind::Stick)
        {
            switch (capture.stickZone)
            {
                case StickZone::DpadUp:    buttons |= XAMINPUT_GAMEPAD_DPAD_UP; break;
                case StickZone::DpadDown:  buttons |= XAMINPUT_GAMEPAD_DPAD_DOWN; break;
                case StickZone::DpadLeft:  buttons |= XAMINPUT_GAMEPAD_DPAD_LEFT; break;
                case StickZone::DpadRight: buttons |= XAMINPUT_GAMEPAD_DPAD_RIGHT; break;

                case StickZone::Stick:
                {
                    ImVec2 vector = GetStickVector(GetControlFrame(capture.control), capture);
                    leftX = ToAxis(vector.x);
                    leftY = ToAxis(-vector.y);
                    break;
                }
            }
        }
        else if (ControlContainsPoint(capture.control, capture.current))
        {
            buttons |= control.buttons;
            leftTrigger = std::max(leftTrigger, control.leftTrigger);
            rightTrigger = std::max(rightTrigger, control.rightTrigger);
        }
    }

    // Keep quick taps pressed for long enough that the game sees them even at low frame rates.
    for (size_t i = 0; i < CONTROL_COUNT; i++)
    {
        if (g_controls[i].kind == TouchControlKind::Button && (time - g_buttonPressTimes[i]) < BUTTON_TAP_HOLD_SECONDS)
        {
            buttons |= g_controls[i].buttons;
            leftTrigger = std::max(leftTrigger, g_controls[i].leftTrigger);
            rightTrigger = std::max(rightTrigger, g_controls[i].rightTrigger);
        }
    }

    // Swipes produce a short look impulse that fades out, so the camera stops when the finger does.
    if (g_lookTime >= 0.0)
    {
        double age = time - g_lookTime;

        if (age < LOOK_HOLD_SECONDS)
        {
            float decay = float(1.0 - age / LOOK_HOLD_SECONDS);
            rightX = ToAxis(g_lookVector.x * decay);
            rightY = ToAxis(g_lookVector.y * decay);
        }
    }

    pad.wButtons |= buttons;
    pad.bLeftTrigger = std::max(pad.bLeftTrigger, leftTrigger);
    pad.bRightTrigger = std::max(pad.bRightTrigger, rightTrigger);
    MergeAxis(pad.sThumbLX, leftX);
    MergeAxis(pad.sThumbLY, leftY);
    MergeAxis(pad.sThumbRX, rightX);
    MergeAxis(pad.sThumbRY, rightY);
}

void TouchControls::Draw(float swapChainWidth, float swapChainHeight, float viewportOffsetX, float viewportOffsetY)
{
    if (!IsActive())
        return;

    std::lock_guard lock(g_mutex);

    auto drawList = ImGui::GetBackgroundDrawList();
    float scale = swapChainWidth / g_windowSize.x;
    float opacity = std::clamp(Config::TouchControlsOpacity.Value, 0.0f, 1.0f);
    float borderWidth = std::max(1.5f * scale, 1.0f);
    bool isEditing = g_isEditing;

    auto toScreen = [&](ImVec2 point)
    {
        return ImVec2(point.x * scale - viewportOffsetX, point.y * scale - viewportOffsetY);
    };

    // White with the given alpha, scaled by the opacity setting unless told otherwise.
    auto white = [&](float alpha, bool applyOpacity = true)
    {
        float finalAlpha = std::clamp(applyOpacity ? alpha * opacity : alpha, 0.0f, 1.0f);
        return IM_COL32(255, 255, 255, int(finalAlpha * 255.0f));
    };

    auto drawLabel = [&](const char* text, ImVec2 centre, float fontSize, ImU32 colour)
    {
        if (g_font == nullptr)
            return;

        ImVec2 textSize = g_font->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, text);
        drawList->AddText(g_font, fontSize, { centre.x - textSize.x * 0.5f, centre.y - textSize.y * 0.5f }, colour, text);
    };

    auto drawPill = [&](const TouchRect& frame, ImU32 fill, ImU32 border)
    {
        ImVec2 min = toScreen({ frame.x, frame.y });
        ImVec2 max = toScreen({ frame.x + frame.width, frame.y + frame.height });
        float rounding = GetRadius(frame) * scale;

        drawList->AddRectFilled(min, max, fill, rounding);
        drawList->AddRect(min, max, border, rounding, 0, borderWidth);
    };

    auto isControlHeld = [&](size_t index)
    {
        return std::any_of(g_captures.begin(), g_captures.end(), [&](const TouchCapture& c) { return c.control == int(index); });
    };

    if (isEditing)
    {
        // Dim the game so it's clear the controls are being edited, not played.
        drawList->AddRectFilled(toScreen({ 0.0f, 0.0f }), toScreen(g_windowSize), IM_COL32(0, 0, 0, 110));
        drawLabel("Drag controls to move them. Tap one to select it.", toScreen({ g_windowSize.x * 0.5f, g_windowSize.y * 0.79f }), g_windowSize.y * 0.045f * scale, IM_COL32(255, 255, 255, 230));
    }
    else
    {
        TouchRect frame = ToWindowFrame(EDIT_BUTTON_FRAME, g_windowSize);
        drawPill(frame, white(0.08f), white(0.32f));
        drawLabel("EDIT", toScreen(GetCentre(frame)), std::min(frame.width, frame.height) * 0.30f * scale, white(0.85f));
    }

    for (size_t i = 0; i < CONTROL_COUNT; i++)
    {
        const TouchControl& control = g_controls[i];
        const LayoutEntry& entry = g_layout[i];

        if (entry.hidden && !isEditing)
            continue;

        TouchRect frame = GetControlFrame(i);
        ImVec2 centre = toScreen(GetCentre(frame));
        bool held = !isEditing && isControlHeld(i);
        bool selected = isEditing && int(i) == g_selectedControl;

        // Hidden controls stay faintly visible in the editor so they can be shown again.
        float alphaScale = entry.hidden ? 0.35f : 1.0f;

        if (control.kind == TouchControlKind::Stick)
        {
            float radius = GetRadius(frame) * scale;
            float ringRadius = std::min(frame.width, frame.height) * STICK_DPAD_RING_RADIUS * scale;

            drawList->AddCircleFilled(centre, radius, white(0.10f * alphaScale), 48);
            drawList->AddCircle(centre, radius, white(0.38f * alphaScale), 48, borderWidth);
            drawList->AddCircle(centre, ringRadius, white(0.20f * alphaScale), 48, borderWidth);

            // D-Pad arrows on the outer ring.
            float arrowDistance = (radius + ringRadius) * 0.5f;
            float arrowSize = (radius - ringRadius) * 0.3f;
            const ImVec2 directions[] = { { 0.0f, -1.0f }, { 0.0f, 1.0f }, { -1.0f, 0.0f }, { 1.0f, 0.0f } };
            const StickZone zones[] = { StickZone::DpadUp, StickZone::DpadDown, StickZone::DpadLeft, StickZone::DpadRight };

            for (size_t d = 0; d < 4; d++)
            {
                bool pressed = !isEditing && std::any_of(g_captures.begin(), g_captures.end(), [&](const TouchCapture& c) { return c.control == int(i) && c.stickZone == zones[d]; });
                ImVec2 dir = directions[d];
                ImVec2 tip = { centre.x + dir.x * (arrowDistance + arrowSize), centre.y + dir.y * (arrowDistance + arrowSize) };
                ImVec2 base = { centre.x + dir.x * (arrowDistance - arrowSize), centre.y + dir.y * (arrowDistance - arrowSize) };
                ImVec2 side = { -dir.y * arrowSize, dir.x * arrowSize };

                drawList->AddTriangleFilled(tip, { base.x + side.x, base.y + side.y }, { base.x - side.x, base.y - side.y }, white((pressed ? 0.85f : 0.45f) * alphaScale));
            }

            // Knob follows the finger while the stick is held.
            ImVec2 knob = centre;
            for (auto& capture : g_captures)
            {
                if (!isEditing && capture.control == int(i) && capture.stickZone == StickZone::Stick)
                {
                    float outerRadius = std::min(frame.width, frame.height) * STICK_ACTIVATION_RADIUS;
                    ImVec2 delta = { capture.current.x - capture.anchor.x, capture.current.y - capture.anchor.y };
                    float distance = std::hypot(delta.x, delta.y);

                    if (distance > outerRadius && distance > 0.0f)
                    {
                        delta.x *= outerRadius / distance;
                        delta.y *= outerRadius / distance;
                    }

                    knob = { centre.x + delta.x * scale, centre.y + delta.y * scale };
                }
            }

            float knobRadius = std::min(frame.width, frame.height) * 0.14f * scale;
            drawList->AddCircleFilled(knob, knobRadius, white(0.18f * alphaScale), 32);
            drawList->AddCircle(knob, knobRadius, white(0.72f * alphaScale), 32, borderWidth);

            if (selected)
                drawList->AddCircle(centre, radius + borderWidth * 3.0f, IM_COL32(255, 200, 40, 255), 48, borderWidth * 2.0f);
        }
        else
        {
            drawPill(frame, white((held ? 0.35f : 0.08f) * alphaScale), white((held ? 0.80f : 0.32f) * alphaScale));

            float fontSize = std::min(frame.width, frame.height) * (strlen(control.label) > 2 ? 0.30f : 0.42f) * scale;
            drawLabel(control.label, centre, fontSize, white((held ? 1.0f : 0.85f) * alphaScale));

            if (selected)
            {
                float outset = borderWidth * 3.0f / scale;
                TouchRect outline = { frame.x - outset, frame.y - outset, frame.width + outset * 2.0f, frame.height + outset * 2.0f };
                ImVec2 min = toScreen({ outline.x, outline.y });
                ImVec2 max = toScreen({ outline.x + outline.width, outline.y + outline.height });
                drawList->AddRect(min, max, IM_COL32(255, 200, 40, 255), GetRadius(outline) * scale, 0, borderWidth * 2.0f);
            }
        }
    }

    if (isEditing)
    {
        // The toolbar ignores the opacity setting so it's always readable.
        for (size_t i = 0; i < EDITOR_BUTTON_COUNT; i++)
        {
            const EditorButton& button = g_editorButtons[i];
            TouchRect frame = GetEditorButtonFrame(i);
            bool needsSelection = button.action == EditorAction::Smaller || button.action == EditorAction::Bigger || button.action == EditorAction::ToggleHidden;
            bool enabled = !needsSelection || g_selectedControl >= 0;
            bool isDone = button.action == EditorAction::Done;
            const char* label = button.label;

            if (button.action == EditorAction::ToggleHidden && g_selectedControl >= 0 && g_layout[g_selectedControl].hidden)
                label = "SHOW";

            ImU32 fill = isDone ? IM_COL32(255, 200, 40, 200) : white(enabled ? 0.25f : 0.08f, false);
            ImU32 text = isDone ? IM_COL32(0, 0, 0, 255) : white(enabled ? 1.0f : 0.35f, false);
            ImVec2 min = toScreen({ frame.x, frame.y });
            ImVec2 max = toScreen({ frame.x + frame.width, frame.y + frame.height });
            float rounding = frame.height * 0.25f * scale;

            drawList->AddRectFilled(min, max, fill, rounding);
            drawList->AddRect(min, max, white(0.6f, false), rounding, 0, borderWidth);
            drawLabel(label, toScreen(GetCentre(frame)), frame.height * 0.32f * scale, text);
        }
    }
}
