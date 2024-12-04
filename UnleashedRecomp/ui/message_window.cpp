#include "message_window.h"
#include "imgui_utils.h"
#include <api/SWA.h>
#include <gpu/video.h>
#include <exports.h>
#include <decompressor.h>
#include <res/images/common/select_fade.dds.h>

constexpr double OVERLAY_CONTAINER_COMMON_MOTION_START = 0;
constexpr double OVERLAY_CONTAINER_COMMON_MOTION_END = 11;
constexpr double OVERLAY_CONTAINER_INTRO_FADE_START = 5;
constexpr double OVERLAY_CONTAINER_INTRO_FADE_END = 9;
constexpr double OVERLAY_CONTAINER_OUTRO_FADE_START = 0;
constexpr double OVERLAY_CONTAINER_OUTRO_FADE_END = 4;

static bool g_isAwaitingResult = false;
static bool g_isClosing = false;
static bool g_isControlsVisible = false;

static int g_selectedRowIndex;
static int g_foregroundCount;

static bool g_upWasHeld;
static bool g_downWasHeld;

static double g_appearTime;
static double g_controlsAppearTime;

static ImFont* g_fntSeurat;

static std::unique_ptr<GuestTexture> g_upSelectionCursor;

std::string g_text;
int g_result;
std::vector<std::string> g_buttons;
int g_defaultButtonIndex;

bool DrawContainer(float appearTime, ImVec2 centre, ImVec2 max, bool isForeground = true)
{
    auto drawList = ImGui::GetForegroundDrawList();

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

    auto vertices = GetPauseContainerVertices(_min, _max);

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
        drawList->AddRectFilled({ 0.0f, 0.0f }, ImGui::GetIO().DisplaySize, IM_COL32(0, 0, 0, 223 * (g_foregroundCount ? 1 : alpha)));

    auto colShadow = IM_COL32(0, 0, 0, 156 * alpha);
    auto colGradientTop = IM_COL32(197, 194, 197, 200 * alpha);
    auto colGradientBottom = IM_COL32(115, 113, 115, 236 * alpha);

    // Draw vertices with gradient.
    SetGradient(_min, _max, colGradientTop, colGradientBottom);
    drawList->AddConvexPolyFilled(vertices.data(), vertices.size(), IM_COL32(255, 255, 255, 255 * alpha));
    ResetGradient();

    // Draw outline.
    drawList->AddPolyline
    (
        vertices.data(),
        vertices.size(),
        IM_COL32(247, 247, 247, 255 * alpha),
        true,
        Scale(2.5f)
    );

    // Offset vertices to draw 3D effect lines.
    for (int i = 0; i < vertices.size(); i++)
    {
        vertices[i].x -= Scale(0.4f);
        vertices[i].y -= Scale(0.2f);
    }

    auto colLineTop = IM_COL32(165, 170, 165, 230 * alpha);
    auto colLineBottom = IM_COL32(190, 190, 190, 230 * alpha);
    auto lineThickness = Scale(1.0f);

    // Top left corner bottom to top left corner top.
    drawList->AddLine(vertices[0], vertices[1], colLineTop, lineThickness * 0.5f);

    // Top left corner bottom to bottom left.
    drawList->AddRectFilledMultiColor({ vertices[0].x - 0.2f, vertices[0].y }, { vertices[6].x + lineThickness - 0.2f, vertices[6].y }, colLineTop, colLineTop, colLineBottom, colLineBottom);

    // Top left corner top to top right.
    drawList->AddLine(vertices[1], vertices[2], colLineTop, lineThickness);

    drawList->PushClipRect(_min, _max);

    return containerMotion >= 1.0f && !g_isClosing;
}

void DrawButton(int rowIndex, float yOffset, float width, float height, std::string& text)
{
    auto drawList = ImGui::GetForegroundDrawList();

    auto clipRectMin = drawList->GetClipRectMin();
    auto clipRectMax = drawList->GetClipRectMax();

    ImVec2 min = { clipRectMin.x + ((clipRectMax.x - clipRectMin.x) - width) / 2, clipRectMin.y + height * rowIndex + yOffset };
    ImVec2 max = { min.x + width, min.y + height };

    bool isSelected = rowIndex == g_selectedRowIndex;

    if (isSelected)
    {
        static auto breatheStart = ImGui::GetTime();
        auto alpha = Lerp(1.0f, 0.75f, (sin((ImGui::GetTime() - breatheStart) * (2.0f * M_PI / (55.0f / 60.0f))) + 1.0f) / 2.0f);
        auto colour = IM_COL32(255, 255, 255, 255 * alpha);

        auto width  = Scale(11);
        auto left   = PIXELS_TO_UV_COORDS(64, 64, 0, 0, 11, 50);
        auto centre = PIXELS_TO_UV_COORDS(64, 64, 11, 0, 8, 50);
        auto right  = PIXELS_TO_UV_COORDS(64, 64, 19, 0, 11, 50);

        drawList->AddImage(g_upSelectionCursor.get(), min, { min.x + width, max.y }, GET_UV_COORDS(left), colour);
        drawList->AddImage(g_upSelectionCursor.get(), { min.x + width, min.y }, { max.x - width, max.y }, GET_UV_COORDS(centre), colour);
        drawList->AddImage(g_upSelectionCursor.get(), { max.x - width, min.y }, max, GET_UV_COORDS(right), colour);
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

static void ResetSelection()
{
    g_selectedRowIndex = g_defaultButtonIndex;
    g_upWasHeld = false;
    g_downWasHeld = false;
}

void MessageWindow::Init()
{
    auto& io = ImGui::GetIO();

    constexpr float FONT_SCALE = 2.0f;

    g_fntSeurat = io.Fonts->AddFontFromFileTTF("FOT-SeuratPro-M.otf", 28.0f * FONT_SCALE);

    g_upSelectionCursor = LoadTexture(decompressZstd(g_select_fade, g_select_fade_uncompressed_size).get(), g_select_fade_uncompressed_size);
}

void MessageWindow::Draw()
{
    if (!s_isVisible)
        return;

    auto pInputState = SWA::CInputState::GetInstance();
    auto drawList = ImGui::GetForegroundDrawList();
    auto& res = ImGui::GetIO().DisplaySize;

    ImVec2 centre = { res.x / 2, res.y / 2 };

    auto fontSize = Scale(28);
    auto textSize = MeasureCentredParagraph(g_fntSeurat, fontSize, 5, g_text.c_str());
    auto textMarginX = Scale(32);
    auto textMarginY = Scale(40);

    if (DrawContainer(g_appearTime, centre, { textSize.x / 2 + textMarginX, textSize.y / 2 + textMarginY }, !g_isControlsVisible))
    {
        DrawCentredParagraph
        (
            g_fntSeurat,
            fontSize,
            { centre.x, centre.y + Scale(3) },
            5,
            g_text.c_str(),

            [=](const char* str, ImVec2 pos)
            {
                DrawTextWithShadow(g_fntSeurat, fontSize, pos, IM_COL32(255, 255, 255, 255), str);
            }
        );

        drawList->PopClipRect();

        if (g_buttons.size())
        {
            auto itemWidth = std::max(Scale(162), Scale(CalcWidestTextSize(g_fntSeurat, fontSize, g_buttons)));
            auto itemHeight = Scale(57);
            auto windowMarginX = Scale(18);
            auto windowMarginY = Scale(25);

            ImVec2 controlsMax = { /* X */ itemWidth / 2 + windowMarginX, /* Y */ itemHeight / 2 * g_buttons.size() + windowMarginY };

            if (g_isControlsVisible && DrawContainer(g_controlsAppearTime, centre, controlsMax))
            {
                auto rowCount = 0;

                for (auto& button : g_buttons)
                    DrawButton(rowCount++, windowMarginY, itemWidth, itemHeight, button);

                drawList->PopClipRect();

                bool upIsHeld = pInputState->GetPadState().IsDown(SWA::eKeyState_DpadUp) ||
                    pInputState->GetPadState().LeftStickVertical > 0.5f;

                bool downIsHeld = pInputState->GetPadState().IsDown(SWA::eKeyState_DpadDown) ||
                    pInputState->GetPadState().LeftStickVertical < -0.5f;

                bool scrollUp = !g_upWasHeld && upIsHeld;
                bool scrollDown = !g_downWasHeld && downIsHeld;

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
                    Game_PlaySound("sys_actstg_pausecursor");

                g_upWasHeld = upIsHeld;
                g_downWasHeld = downIsHeld;

                if (pInputState->GetPadState().IsTapped(SWA::eKeyState_A))
                {
                    g_result = g_selectedRowIndex;

                    Game_PlaySound("sys_actstg_pausedecide");
                    MessageWindow::Close();
                }
                else if (pInputState->GetPadState().IsTapped(SWA::eKeyState_B))
                {
                    g_result = -1;

                    Game_PlaySound("sys_actstg_pausecansel");
                    MessageWindow::Close();
                }
            }
            else
            {
                if (!g_isControlsVisible && pInputState->GetPadState().IsTapped(SWA::eKeyState_A))
                {
                    g_controlsAppearTime = ImGui::GetTime();
                    g_isControlsVisible = true;

                    Game_PlaySound("sys_actstg_pausewinopen");
                }
            }
        }
        else
        {
            if (pInputState->GetPadState().IsTapped(SWA::eKeyState_A))
                MessageWindow::Close();
        }
    }
}

bool MessageWindow::Open(std::string text, int* result, std::span<std::string> buttons, int defaultButtonIndex)
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

        ResetSelection();

        Game_PlaySound("sys_actstg_pausewinopen");

        g_isAwaitingResult = true;
    }

    *result = g_result;

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
        g_isAwaitingResult = false;
    }

    Game_PlaySound("sys_actstg_pausewinclose");
}
