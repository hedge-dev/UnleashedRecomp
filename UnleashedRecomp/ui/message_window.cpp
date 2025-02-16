#include "message_window.h"
#include <api/SWA.h>
#include <gpu/imgui/imgui_snapshot.h>
#include <gpu/video.h>
#include <hid/hid.h>
#include <locale/locale.h>
#include <ui/button_guide.h>
#include <ui/imgui_utils.h>
#include <app.h>
#include <decompressor.h>
#include <exports.h>
#include <sdl_listener.h>

constexpr double OVERLAY_CONTAINER_COMMON_MOTION_START = 0;
constexpr double OVERLAY_CONTAINER_COMMON_MOTION_END = 11;
constexpr double OVERLAY_CONTAINER_INTRO_FADE_START = 5;
constexpr double OVERLAY_CONTAINER_INTRO_FADE_END = 9;
constexpr double OVERLAY_CONTAINER_OUTRO_FADE_START = 0;
constexpr double OVERLAY_CONTAINER_OUTRO_FADE_END = 4;

static bool g_isAwaitingResult = false;
static bool g_isClosing = false;
static bool g_isControlsVisible = false;

static double g_rowSelectionTime;
static int g_selectedRowIndex;
static int g_prevSelectedRowIndex;
static int g_foregroundCount;

static bool g_upWasHeld;
static bool g_downWasHeld;

static ImVec2 g_joypadAxis = {};
static bool g_isAccepted;
static bool g_isDeclined;

static double g_appearTime;
static double g_controlsAppearTime;

static ImFont* g_fntSeurat;

std::string g_text;
int g_result;
std::vector<std::string> g_buttons;
int g_defaultButtonIndex;
int g_cancelButtonIndex;

class SDLEventListenerForMessageWindow : public SDLEventListener
{
public:
    bool OnSDLEvent(SDL_Event* event) override
    {
        if (App::s_isInit || !MessageWindow::s_isVisible || !hid::IsInputAllowed())
            return false;

        constexpr float axisValueRange = 32767.0f;
        constexpr float axisTapRange = 0.5f;

        ImVec2 tapDirection = {};

        switch (event->type)
        {
            case SDL_KEYDOWN:
            {
                switch (event->key.keysym.scancode)
                {
                    case SDL_SCANCODE_UP:
                        g_joypadAxis.y = -1.0f;
                        break;

                    case SDL_SCANCODE_DOWN:
                        g_joypadAxis.y = 1.0f;
                        break;

                    case SDL_SCANCODE_RETURN:
                    case SDL_SCANCODE_KP_ENTER:
                        g_isAccepted = true;
                        break;

                    case SDL_SCANCODE_ESCAPE:
                        g_isDeclined = true;
                        break;
                }

                break;
            }

            case SDL_MOUSEBUTTONDOWN:
            {
                // Only accept left mouse button.
                if (event->button.button != SDL_BUTTON_LEFT)
                    break;

                // Only accept mouse buttons when an item is selected.
                if (g_isControlsVisible && g_selectedRowIndex == -1)
                    break;

                g_isAccepted = true;

                break;
            }

            case SDL_CONTROLLERBUTTONDOWN:
            {
                switch (event->cbutton.button)
                {
                    case SDL_CONTROLLER_BUTTON_DPAD_UP:
                        g_joypadAxis = { 0.0f, -1.0f };
                        break;

                    case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
                        g_joypadAxis = { 0.0f, 1.0f };
                        break;

                    case SDL_CONTROLLER_BUTTON_A:
                        g_isAccepted = true;
                        break;

                    case SDL_CONTROLLER_BUTTON_B:
                        g_isDeclined = true;
                        break;
                }

                break;
            }

            case SDL_CONTROLLERAXISMOTION:
            {
                if (event->caxis.axis < 2)
                {
                    float newAxisValue = event->caxis.value / axisValueRange;
                    bool sameDirection = (newAxisValue * g_joypadAxis[event->caxis.axis]) > 0.0f;
                    bool wasInRange = abs(g_joypadAxis[event->caxis.axis]) > axisTapRange;
                    bool isInRange = abs(newAxisValue) > axisTapRange;

                    if (sameDirection && !wasInRange && isInRange)
                        tapDirection[event->caxis.axis] = newAxisValue;

                    g_joypadAxis[event->caxis.axis] = newAxisValue;
                }

                break;
            }
        }

        return false;
    }
}
g_sdlEventListenerForMessageWindow;

bool DrawContainer(float appearTime, ImVec2 centre, ImVec2 max, bool isForeground = true)
{
    auto drawList = ImGui::GetBackgroundDrawList();

    ImVec2 _min = { centre.x - max.x, centre.y - max.y };
    ImVec2 _max = { centre.x + max.x, centre.y + max.y };

    // Expand/retract animation.
    auto containerMotion = ComputeMotion(appearTime, OVERLAY_CONTAINER_COMMON_MOTION_START, OVERLAY_CONTAINER_COMMON_MOTION_END);

    if (g_isClosing)
    {
        _min.x = Hermite(_min.x, centre.x, containerMotion);
        _max.x = Hermite(_max.x, centre.x, containerMotion);
        _min.y = Hermite(_min.y, centre.y, containerMotion);
        _max.y = Hermite(_max.y, centre.y, containerMotion);
    }
    else
    {
        _min.x = Hermite(centre.x, _min.x, containerMotion);
        _max.x = Hermite(centre.x, _max.x, containerMotion);
        _min.y = Hermite(centre.y, _min.y, containerMotion);
        _max.y = Hermite(centre.y, _max.y, containerMotion);
    }

    // Transparency fade animation.
    auto colourMotion = g_isClosing
        ? ComputeMotion(appearTime, OVERLAY_CONTAINER_OUTRO_FADE_START, OVERLAY_CONTAINER_OUTRO_FADE_END)
        : ComputeMotion(appearTime, OVERLAY_CONTAINER_INTRO_FADE_START, OVERLAY_CONTAINER_INTRO_FADE_END);

    auto alpha = g_isClosing
        ? Lerp(1, 0, colourMotion)
        : Lerp(0, 1, colourMotion);

    if (!isForeground)
        g_foregroundCount++;

    if (isForeground)
        drawList->AddRectFilled({ 0.0f, 0.0f }, ImGui::GetIO().DisplaySize, IM_COL32(0, 0, 0, 190 * (g_foregroundCount ? 1 : alpha)));

    DrawPauseContainer(_min, _max, alpha);

    if (containerMotion >= 1.0f && !g_isClosing)
    {
        drawList->PushClipRect(_min, _max);
        return true;
    }

    return false;
}

void DrawButton(int rowIndex, float yOffset, float width, float height, std::string& text)
{
    auto drawList = ImGui::GetBackgroundDrawList();

    auto clipRectMin = drawList->GetClipRectMin();
    auto clipRectMax = drawList->GetClipRectMax();

    ImVec2 min = { clipRectMin.x + ((clipRectMax.x - clipRectMin.x) - width) / 2, clipRectMin.y + height * rowIndex + yOffset };
    ImVec2 max = { min.x + width, min.y + height };

    bool isSelected = rowIndex == g_selectedRowIndex;

    if (isSelected)
    {
        auto prevItemOffset = (g_prevSelectedRowIndex - g_selectedRowIndex) * height;
        auto animRatio = std::clamp((ImGui::GetTime() - g_rowSelectionTime) * 60.0 / 8.0, 0.0, 1.0);
        prevItemOffset *= pow(1.0 - animRatio, 3.0);

        DrawSelectionContainer({ min.x, min.y + prevItemOffset }, { max.x, max.y + prevItemOffset }, true);
    }

    auto fontSize = Scale(28);
    auto textSize = g_fntSeurat->CalcTextSizeA(fontSize, FLT_MAX, 0, text.c_str());

    DrawTextWithShadow
    (
        g_fntSeurat,
        fontSize,
        { /* X */ min.x + ((max.x - min.x) - textSize.x) / 2, /* Y */ min.y + ((max.y - min.y) - textSize.y) / 2 },
        isSelected ? IM_COL32(255, 128, 0, 255) : IM_COL32(255, 255, 255, 255),
        text.c_str()
    );
}

void DrawNextButtonGuide(bool isController, bool isKeyboard)
{
    auto icon = isController
        ? EButtonIcon::A
        : isKeyboard
            ? EButtonIcon::Enter
            : EButtonIcon::LMB;

    // Always show controller prompt in-game.
    if (App::s_isInit)
        icon = EButtonIcon::A;

    ButtonGuide::Open(Button("Common_Next", icon));
}

static void ResetSelection()
{
    /* Always use -1 for mouse input to prevent the selection
       cursor from erroneously appearing where it shouldn't. */
    g_selectedRowIndex = hid::g_inputDevice == hid::EInputDevice::Mouse
        ? -1
        : g_defaultButtonIndex;

    g_upWasHeld = false;
    g_downWasHeld = false;
    g_joypadAxis = {};
    g_isAccepted = false;
    g_isDeclined = false;
}

void MessageWindow::Init()
{
    auto& io = ImGui::GetIO();

    g_fntSeurat = ImFontAtlasSnapshot::GetFont("FOT-SeuratPro-M.otf");
}

void MessageWindow::Draw()
{
    if (!s_isVisible)
        return;

    auto drawList = ImGui::GetBackgroundDrawList();
    auto& res = ImGui::GetIO().DisplaySize;

    ImVec2 centre = { res.x / 2, res.y / 2 };

    auto maxWidth = Scale(820);
    auto fontSize = Scale(28);

    const auto input = RemoveRubyAnnotations(g_text.c_str());
    auto lines = Split(input.first.c_str(), g_fntSeurat, fontSize, maxWidth);
    
    for (auto& line : lines)
    {
        line = ReAddRubyAnnotations(line, input.second);
    }

    auto lineMargin = Config::Language != ELanguage::Japanese ? 5.0f : 5.5f;
    auto textSize = MeasureCentredParagraph(g_fntSeurat, fontSize, lineMargin, lines);
    auto textMarginX = Scale(37);
    auto textMarginY = Scale(45);

    auto textX = centre.x;
    auto textY = centre.y + Scale(3);

    if (Config::Language == ELanguage::Japanese)
    {
        textMarginX -= Scale(2.5f);
        textMarginY -= Scale(2.0f);

        textY += Scale(lines.size() % 2 == 0 ? 1.5f : 8.0f);
    }
    
    bool isController = hid::IsInputDeviceController();
    bool isKeyboard = hid::g_inputDevice == hid::EInputDevice::Keyboard;

    // Handle controller input when the game is booted.
    if (App::s_isInit)
    {
        // Always assume keyboard to prevent mouse from blocking control in-game.
        isKeyboard = true;

        if (auto pInputState = SWA::CInputState::GetInstance())
        {
            auto& rPadState = pInputState->GetPadState();

            g_joypadAxis.y = rPadState.LeftStickVertical;

            if (rPadState.IsTapped(SWA::eKeyState_DpadUp))
                g_joypadAxis.y = -1.0f;

            if (rPadState.IsTapped(SWA::eKeyState_DpadDown))
                g_joypadAxis.y = 1.0f;

            g_isAccepted = rPadState.IsTapped(SWA::eKeyState_A);
            g_isDeclined = rPadState.IsTapped(SWA::eKeyState_B);

            if (isKeyboard)
                g_isAccepted = g_isAccepted || rPadState.IsTapped(SWA::eKeyState_Start);
        }
    }

    if (DrawContainer(g_appearTime, centre, { textSize.x / 2 + textMarginX, textSize.y / 2 + textMarginY }, !g_isControlsVisible))
    {
        DrawRubyAnnotatedText
        (
            g_fntSeurat,
            fontSize,
            maxWidth,
            { textX, textY },
            lineMargin,
            g_text.c_str(),

            [=](const char* str, ImVec2 pos)
            {
                DrawTextWithShadow(g_fntSeurat, fontSize, pos, IM_COL32(255, 255, 255, 255), str);
            },
            [=](const char* str, float size, ImVec2 pos)
            {
                DrawTextWithShadow(g_fntSeurat, size, pos, IM_COL32(255, 255, 255, 255), str, 1.0f);
            },

            true
        );

        drawList->PopClipRect();

        if (g_buttons.size())
        {
            auto itemWidth = std::max(Scale(162), Scale(CalcWidestTextSize(g_fntSeurat, fontSize, g_buttons)));
            auto itemHeight = Scale(57);
            auto windowMarginX = Scale(23);
            auto windowMarginY = Scale(30);

            ImVec2 controlsMax = { /* X */ itemWidth / 2 + windowMarginX, /* Y */ itemHeight / 2 * g_buttons.size() + windowMarginY };

            if (g_isControlsVisible && DrawContainer(g_controlsAppearTime, centre, controlsMax))
            {
                auto rowCount = 0;

                for (auto& button : g_buttons)
                    DrawButton(rowCount++, windowMarginY, itemWidth, itemHeight, button);

                if (isController || isKeyboard)
                {
                    bool upIsHeld = g_joypadAxis.y > 0.5f;
                    bool downIsHeld = g_joypadAxis.y < -0.5f;

                    bool scrollUp = !g_upWasHeld && upIsHeld;
                    bool scrollDown = !g_downWasHeld && downIsHeld;

                    auto prevSelectedRowIndex = g_selectedRowIndex;

                    if (scrollUp)
                    {
                        --g_selectedRowIndex;
                        if (g_selectedRowIndex < 0)
                            g_selectedRowIndex = rowCount - 1;
                    }
                    else if (scrollDown)
                    {
                        ++g_selectedRowIndex;
                        if (g_selectedRowIndex >= rowCount)
                            g_selectedRowIndex = 0;
                    }

                    if (scrollUp || scrollDown)
                    {
                        Game_PlaySound("sys_actstg_pausecursor");
                        g_rowSelectionTime = ImGui::GetTime();
                        g_prevSelectedRowIndex = prevSelectedRowIndex;
                        g_joypadAxis.y = 0;
                    }

                    g_upWasHeld = upIsHeld;
                    g_downWasHeld = downIsHeld;

                    auto selectIcon = EButtonIcon::A;
                    auto backIcon = EButtonIcon::B;

                    if (isController || isKeyboard)
                    {
                        if (isKeyboard && !App::s_isInit)
                        {
                            // Only display keyboard prompt during installer.
                            selectIcon = EButtonIcon::Enter;
                            backIcon = EButtonIcon::Escape;
                        }

                        std::array<Button, 2> buttons =
                        {
                            Button("Common_Select", selectIcon),
                            Button("Common_Back", backIcon),
                        };

                        ButtonGuide::Open(buttons);
                    }

                    if (g_isDeclined)
                    {
                        g_result = g_cancelButtonIndex;

                        Game_PlaySound("sys_actstg_pausecansel");
                        MessageWindow::Close();
                    }
                }
                else if (!App::s_isInit) // Only accept mouse input during installer.
                {
                    auto clipRectMin = drawList->GetClipRectMin();
                    auto clipRectMax = drawList->GetClipRectMax();

                    ImVec2 listMin = { clipRectMin.x + windowMarginX, clipRectMin.y + windowMarginY };
                    ImVec2 listMax = { clipRectMax.x - windowMarginX, clipRectMin.y + windowMarginY + itemHeight * rowCount };

                    // Invalidate index if the mouse cursor is outside of the list box.
                    if (!ImGui::IsMouseHoveringRect(listMin, listMax, false))
                        g_selectedRowIndex = -1;

                    for (int i = 0; i < rowCount; i++)
                    {
                        ImVec2 itemMin = { listMin.x, listMin.y + itemHeight * i };
                        ImVec2 itemMax = { listMax.x, clipRectMin.y + windowMarginY + itemHeight * i + itemHeight };

                        if (ImGui::IsMouseHoveringRect(itemMin, itemMax, false))
                        {
                            if (g_selectedRowIndex != i)
                                Game_PlaySound("sys_actstg_pausecursor");

                            g_selectedRowIndex = i;

                            break;
                        }
                    }

                    std::array<Button, 2> buttons =
                    {
                        Button("Common_Select", EButtonIcon::LMB),
                        Button("Common_Back", EButtonIcon::Escape),
                    };

                    ButtonGuide::Open(buttons);
                }

                if (g_selectedRowIndex != -1 && g_isAccepted)
                {
                    g_result = g_selectedRowIndex;

                    Game_PlaySound("sys_actstg_pausedecide");
                    MessageWindow::Close();
                }

                drawList->PopClipRect();
            }
            else
            {
                DrawNextButtonGuide(isController, isKeyboard);

                if (!g_isControlsVisible && g_isAccepted)
                {
                    g_controlsAppearTime = ImGui::GetTime();
                    g_isControlsVisible = true;

                    ResetSelection();
                    Game_PlaySound("sys_actstg_pausewinopen");
                }
            }
        }
        else
        {
            DrawNextButtonGuide(isController, isKeyboard);

            if (g_isAccepted)
            {
                g_result = 0;

                MessageWindow::Close();
            }
        }
    }
    else if (g_isClosing)
    {
        s_isVisible = false;
    }
}

bool MessageWindow::Open(std::string text, int* result, std::span<std::string> buttons, int defaultButtonIndex, int cancelButtonIndex)
{
    if (!g_isAwaitingResult && *result == -1)
    {
        s_isVisible = true;
        g_isClosing = false;
        g_isControlsVisible = false;
        g_foregroundCount = 0;
        g_appearTime = ImGui::GetTime();
        g_controlsAppearTime = ImGui::GetTime();

        g_text = text;
        g_buttons = std::vector(buttons.begin(), buttons.end());
        g_defaultButtonIndex = defaultButtonIndex;
        g_cancelButtonIndex = cancelButtonIndex;

        ResetSelection();

        Game_PlaySound("sys_actstg_pausewinopen");

        g_isAwaitingResult = true;
    }

    *result = g_result;

    // Returns true when the message window is closed.
    return !g_isAwaitingResult;
}

void MessageWindow::Close()
{
    if (!g_isClosing)
    {
        g_appearTime = ImGui::GetTime();
        g_controlsAppearTime = ImGui::GetTime();
        g_isClosing = true;
        g_isControlsVisible = false;
        g_foregroundCount = 0;
        g_isAwaitingResult = false;

        ButtonGuide::Close();
    }

    Game_PlaySound("sys_actstg_pausewinclose");
}
