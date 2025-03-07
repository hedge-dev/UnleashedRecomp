#include "options_menu.h"
#include "options_menu_thumbnails.h"
#include "tv_static.h"

#include <api/SWA.h>
#include <apu/audio.h>
#include <gpu/imgui/imgui_common.h>
#include <gpu/video.h>
#include <gpu/imgui/imgui_snapshot.h>
#include <hid/hid.h>
#include <kernel/heap.h>
#include <kernel/memory.h>
#include <locale/locale.h>
#include <patches/aspect_ratio_patches.h>
#include <patches/audio_patches.h>
#include <ui/button_guide.h>
#include <ui/game_window.h>
#include <ui/imgui_utils.h>
#include <app.h>
#include <decompressor.h>
#include <exports.h>
#include <sdl_listener.h>

#include <res/images/options_menu/miles_electric.dds.h>

static constexpr double MILES_ELECTRIC_SCALE_DURATION = 16.0;
static constexpr double MILES_ELECTRIC_BACKGROUND_DURATION = 16.0;
static constexpr double MILES_ELECTRIC_FOREGROUND_FADE_DURATION = 6.0;
static constexpr double MILES_ELECTRIC_FOREGROUND_FADE_IN_TIME = MILES_ELECTRIC_SCALE_DURATION - 2.0;
static constexpr double MILES_ELECTRIC_FOREGROUND_FADE_OUT_TIME = MILES_ELECTRIC_FOREGROUND_FADE_IN_TIME + MILES_ELECTRIC_FOREGROUND_FADE_DURATION;

static constexpr double VALUE_SLIDER_INTRO_DURATION = 20.0;

static constexpr double CONTAINER_LINE_ANIMATION_DURATION = 8.0;

static constexpr double CONTAINER_OUTER_TIME = CONTAINER_LINE_ANIMATION_DURATION + 8.0; // 8 frame delay
static constexpr double CONTAINER_OUTER_DURATION = 8.0;

static constexpr double CONTAINER_INNER_TIME = CONTAINER_OUTER_TIME + CONTAINER_OUTER_DURATION + 8.0; // 8 frame delay
static constexpr double CONTAINER_INNER_DURATION = 8.0;

static constexpr double CONTAINER_BACKGROUND_TIME = CONTAINER_INNER_TIME + CONTAINER_INNER_DURATION + 8.0; // 8 frame delay
static constexpr double CONTAINER_BACKGROUND_DURATION = 12.0;

static constexpr double CONTAINER_FULL_DURATION = CONTAINER_BACKGROUND_TIME + CONTAINER_BACKGROUND_DURATION;

static constexpr double CONTAINER_CATEGORY_TIME = (CONTAINER_INNER_TIME + CONTAINER_BACKGROUND_TIME) / 2.0;
static constexpr double CONTAINER_CATEGORY_DURATION = 12.0;

static constexpr float CONTAINER_POS_Y = 117.0f;

static constexpr float SETTINGS_WIDE_GRID_COUNT = 90.0f;
static constexpr float INFO_WIDE_GRID_COUNT = 42.0f;
static constexpr float PADDING_WIDE_GRID_COUNT = 3.0f;

static constexpr float SETTINGS_NARROW_GRID_COUNT = 70.0f;
static constexpr float INFO_NARROW_GRID_COUNT = 34.0f;
static constexpr float PADDING_NARROW_GRID_COUNT = 1.0f;

static constexpr float INFO_TEXT_MARQUEE_DELAY = 1.2f;

static constexpr int32_t g_categoryCount = 4;
static int32_t g_categoryIndex;
static ImVec2 g_categoryAnimMin;
static ImVec2 g_categoryAnimMax;

static int32_t g_firstVisibleRowIndex;
static int32_t g_prevSelectedRowIndex;
static int32_t g_selectedRowIndex;
static double g_rowSelectionTime;
static float g_prevOffsetRatio;

static bool g_leftWasHeld;
static bool g_upWasHeld;
static bool g_rightWasHeld;
static bool g_downWasHeld;

static bool g_lockedOnOption;
static double g_lockedOnTime;
static double g_lastTappedTime;
static double g_lastIncrementTime;
static double g_lastIncrementSoundTime;

static constexpr size_t GRID_SIZE = 9;

static ImFont* g_seuratFont;
static ImFont* g_dfsogeistdFont;
static ImFont* g_newRodinFont;

static const IConfigDef* g_selectedItem;

static std::string* g_inaccessibleReason;

static bool g_isStage = false;
static bool g_isClosing = false;
static bool g_isControlsVisible = false;
static bool g_isEnterKeyBuffered = false;
static bool g_canReset = false;
static bool g_isLanguageOptionChanged = false;
static bool g_titleAnimBegin = true;

static double g_appearTime = 0.0;

static std::unique_ptr<GuestTexture> g_upMilesElectric;

static float g_rightStickY;

class SDLEventListenerForOptionsMenu : public SDLEventListener
{
public:
    bool OnSDLEvent(SDL_Event* event) override
    {
        if (!OptionsMenu::s_isVisible || !hid::IsInputAllowed())
            return false;

        if (event->type == SDL_CONTROLLERAXISMOTION && event->caxis.axis == SDL_CONTROLLER_AXIS_RIGHTY)
            g_rightStickY = event->caxis.value / 32767.0f;

        return false;
    }
}
g_sdlEventListenerForOptionsMenu;

static void DrawTitle()
{
    static constexpr double fadeOffset = 3.0;
    static constexpr double fadeDuration = 28.0;
    static constexpr double fadeAdditiveDuration = 20.0;
    static constexpr double squareMoveDuration = 5.0;
    static constexpr double squareMoveEndDuration = 40.0;
    static constexpr double squareBlinkDuration = 10.0;
    static constexpr double squareFadeDuration = 45.0;

    static bool isRectVisible;
    static bool isRectFinalAdjustment;
    static float rectX;
    static double rectMoveMotionOffset;
    static double rectBlinkMotionOffset;

    if (g_titleAnimBegin)
    {
        isRectVisible = true;
        isRectFinalAdjustment = false;
        rectX = 0;
        rectMoveMotionOffset = 0;
        rectBlinkMotionOffset = 0;
        g_titleAnimBegin = false;
    }

    auto drawList = ImGui::GetBackgroundDrawList();
    auto x = Scale(122);
    auto y = Scale(56);

    if (g_aspectRatio >= WIDE_ASPECT_RATIO)
        x += g_aspectRatioOffsetX;
    else
        x += (1.0f - g_aspectRatioNarrowScale) * g_aspectRatioScale * -20.0f;

    ImVec2 optionsMin = { x, y };

    auto drawText = [&](float alpha)
    {
        DrawTextWithOutline
        (
            g_dfsogeistdFont,
            Scale(48),
            optionsMin,
            IM_COL32(255, 190, 33, 255 * alpha),
            Localise("Options_Header_Name").c_str(),
            4,
            IM_COL32(0, 0, 0, 255 * alpha),
            IMGUI_SHADER_MODIFIER_TITLE_BEVEL
        );
    };

    if (g_isStage)
    {
        drawText(1.0f);
        return;
    }

    drawText(Hermite(0.0f, 1.0f, ComputeMotion(g_appearTime, fadeOffset, fadeDuration)));

    auto rectMoveMotion = ComputeMotion(g_appearTime, rectMoveMotionOffset, squareMoveDuration);
    auto rectEndMotion = ComputeMotion(g_appearTime, 0.0, squareMoveEndDuration);
    auto rectBlinkMotion = sin(ComputeMotion(g_appearTime, squareMoveEndDuration + rectBlinkMotionOffset, squareBlinkDuration) * M_PI);
    auto rectAlphaMotion = 1.0f;
    auto rectY = Scale(10);
    auto rectSize = Scale(32);

    if (rectBlinkMotion > 0.0)
    {
        if (!isRectFinalAdjustment)
        {
            /* Ensure the blinking animation starts
               past the German localisation. */
            rectX += rectSize + (rectSize * 0.25f);
            isRectFinalAdjustment = true;
        }
    
        isRectVisible = !isRectVisible;
        rectBlinkMotionOffset += squareBlinkDuration;
    }

    if (rectEndMotion >= 1.0)
    {
        // Fade out the square.
        rectAlphaMotion = 1.0f - ComputeMotion(g_appearTime, squareMoveEndDuration + squareBlinkDuration, squareFadeDuration);
    }
    else if (rectMoveMotion >= 1.0)
    {
        // Move the square along in steps by its own width and offset the animation to repeat.
        rectX += rectSize;
        rectMoveMotionOffset += squareMoveDuration;
    }

    if (isRectVisible)
    {
        float rectScale = 1.0f;
        if (rectBlinkMotion == 0.0)
        {
            constexpr float RECT_SCALES[] = { 1.2f, 1.1f, 1.1f, 1.0f, 1.15f, 0.4f, 1.2f, 1.1f, 1.05f, 1.0f, 1.5f, 1.2f, 1.0f };
            rectScale = RECT_SCALES[uint32_t(round(rectX / rectSize)) % std::size(RECT_SCALES)];
        }

        ImVec2 rectMin = { optionsMin.x + rectX, optionsMin.y + rectY };
        ImVec2 rectMax = { optionsMin.x + rectX + rectSize * rectScale, optionsMin.y + rectY + rectSize };

        auto rectOutlineMargin = Scale(2.5f);

        ImVec2 rectOutlineMin = { rectMin.x - rectOutlineMargin, rectMin.y - rectOutlineMargin };
        ImVec2 rectOutlineMax = { rectMax.x + rectOutlineMargin, rectMax.y + rectOutlineMargin };

        drawList->AddRectFilled(rectOutlineMin, rectOutlineMax, IM_COL32(0, 0, 0, 255 * rectAlphaMotion), Scale(5));

        SetShaderModifier(IMGUI_SHADER_MODIFIER_RECTANGLE_BEVEL);
        SetGradient(rectMin, rectMax, IM_COL32_WHITE, IM_COL32_WHITE);

        drawList->AddRectFilled(rectMin, rectMax, IM_COL32(255, 188, 0, 255 * rectAlphaMotion));

        ResetGradient();
        SetShaderModifier(IMGUI_SHADER_MODIFIER_NONE);
    }

    // The flash gets rendered after the rectangle in the original game.
    SetAdditive(true);
    drawText(1.0 - 2.0 * abs(ComputeLinearMotion(g_appearTime, fadeDuration, fadeAdditiveDuration) - 0.5));
    SetAdditive(false);
}

static void DrawScanlineBars()
{
    constexpr uint32_t COLOR0 = IM_COL32(203, 255, 0, 0);
    constexpr uint32_t COLOR1 = IM_COL32(203, 255, 0, 55);
    constexpr uint32_t FADE_COLOR0 = IM_COL32(0, 0, 0, 255);
    constexpr uint32_t FADE_COLOR1 = IM_COL32(0, 0, 0, 0);

    float height = Scale(105.0f);

    auto& res = ImGui::GetIO().DisplaySize;
    auto drawList = ImGui::GetBackgroundDrawList();

    if (OptionsMenu::s_pauseMenuType != SWA::eMenuType_WorldMap)
    {
        // Top bar fade
        drawList->AddRectFilledMultiColor
        (
            { 0.0f, 0.0f },
            { res.x, height },
            FADE_COLOR0,
            FADE_COLOR0,
            FADE_COLOR1,
            FADE_COLOR1
        );

        // Bottom bar fade
        drawList->AddRectFilledMultiColor
        (
            { res.x, res.y },
            { 0.0f, res.y - height },
            FADE_COLOR0,
            FADE_COLOR0,
            FADE_COLOR1,
            FADE_COLOR1
        );
    }

    SetShaderModifier(IMGUI_SHADER_MODIFIER_SCANLINE);

    // Top bar
    drawList->AddRectFilledMultiColor
    (
        { 0.0f, 0.0f },
        { res.x, height },
        COLOR0,
        COLOR0,
        COLOR1,
        COLOR1
    );

    // Bottom bar
    ImVec2 max{ 0.0f, res.y - height };
    SetProceduralOrigin(max);

    drawList->AddRectFilledMultiColor
    (
        { res.x, res.y },
        max,
        COLOR0,
        COLOR0,
        COLOR1,
        COLOR1
    );

    ResetProceduralOrigin();

    SetShaderModifier(IMGUI_SHADER_MODIFIER_NONE);

    DrawTitle();

    auto drawLine = [&](bool top)
    {
        float y = top ? height : (res.y - height);

        constexpr uint32_t TOP_COLOR0 = IM_COL32(222, 255, 189, 7);
        constexpr uint32_t TOP_COLOR1 = IM_COL32(222, 255, 189, 65);
        constexpr uint32_t BOTTOM_COLOR0 = IM_COL32(173, 255, 156, 65);
        constexpr uint32_t BOTTOM_COLOR1 = IM_COL32(173, 255, 156, 7);

        drawList->AddRectFilledMultiColor(
            { 0.0f, y - Scale(2.0f) },
            { res.x, y }, 
            top ? TOP_COLOR0 : BOTTOM_COLOR1,
            top ? TOP_COLOR0 : BOTTOM_COLOR1, 
            top ? TOP_COLOR1 : BOTTOM_COLOR0,
            top ? TOP_COLOR1 : BOTTOM_COLOR0);

        drawList->AddRectFilledMultiColor(
            { 0.0f, y + Scale(1.0f) }, 
            { res.x, y + Scale(3.0f) }, 
            top ? BOTTOM_COLOR0 : TOP_COLOR1, 
            top ? BOTTOM_COLOR0 : TOP_COLOR1,
            top ? BOTTOM_COLOR1 : TOP_COLOR0, 
            top ? BOTTOM_COLOR1 : TOP_COLOR0);

        constexpr uint32_t CENTER_COLOR = IM_COL32(115, 178, 104, 255);
        drawList->AddRectFilled({ 0.0f, y }, { res.x, y + Scale(1.0f) }, CENTER_COLOR);
    };

    // Top bar line
    drawLine(true);

    // Bottom bar line
    drawLine(false);

    DrawVersionString(g_newRodinFont);
}

static float AlignToNextGrid(float value)
{
    return round(value / GRID_SIZE) * GRID_SIZE;
}

static void DrawContainer(ImVec2 min, ImVec2 max, bool drawRightOutline)
{
    double containerHeight = g_isStage ? 1.0 : ComputeMotion(g_appearTime, 0.0, CONTAINER_LINE_ANIMATION_DURATION);

    float center = (min.y + max.y) / 2.0f;
    min.y = Lerp(center, min.y, containerHeight);
    max.y = Lerp(center, max.y, containerHeight);

    auto& res = ImGui::GetIO().DisplaySize;
    auto drawList = ImGui::GetBackgroundDrawList();

    double outerAlpha = g_isStage ? 1.0 : ComputeMotion(g_appearTime, CONTAINER_OUTER_TIME, CONTAINER_OUTER_DURATION);
    double innerAlpha = g_isStage ? 1.0 : ComputeMotion(g_appearTime, CONTAINER_INNER_TIME, CONTAINER_INNER_DURATION);
    double backgroundAlpha = g_isStage ? 1.0 : ComputeMotion(g_appearTime, CONTAINER_BACKGROUND_TIME, CONTAINER_BACKGROUND_DURATION);

    const uint32_t lineColor = IM_COL32(0, 89, 0, 255 * containerHeight);
    const uint32_t outerColor = IM_COL32(0, 49, 0, 255 * outerAlpha);
    const uint32_t innerColor = IM_COL32(0, 33, 0, 255 * innerAlpha);
    const uint32_t backgroundColor = IM_COL32(0, 0, 0, 223 * backgroundAlpha);

    float gridSize = Scale(GRID_SIZE);

    drawList->AddRectFilled(min, max, backgroundColor); // Background

    SetShaderModifier(IMGUI_SHADER_MODIFIER_CHECKERBOARD);

    drawList->AddRectFilled({ min.x, min.y + gridSize }, { min.x + gridSize, max.y - gridSize }, outerColor); // Container outline left
    drawList->AddRectFilled({ max.x - gridSize, min.y + gridSize }, { max.x, max.y - gridSize }, drawRightOutline ? outerColor : innerColor); // Container outline right
    drawList->AddRectFilled(min, { max.x, min.y + gridSize }, outerColor); // Container outline top
    drawList->AddRectFilled({ min.x, max.y - gridSize }, max, outerColor); // Container outline bottom

    drawList->AddRectFilled({ min.x + gridSize, min.y + gridSize }, { max.x - gridSize, max.y - gridSize }, innerColor); // Inner container

    SetShaderModifier(IMGUI_SHADER_MODIFIER_NONE);

    float lineSize = Scale(2);

    // Top line
    drawList->AddLine({ min.x + gridSize, min.y + gridSize }, { min.x + gridSize, min.y + gridSize * 2.0f }, lineColor, lineSize); // Vertical left
    drawList->AddLine({ min.x + gridSize, min.y + gridSize }, { max.x - gridSize, min.y + gridSize }, lineColor, lineSize); // Horizontal
    drawList->AddLine({ max.x - gridSize, min.y + gridSize }, { max.x - gridSize, min.y + gridSize * 2.0f }, lineColor, lineSize); // Vertical right

    // Bottom line
    drawList->AddLine({ min.x + gridSize, max.y - gridSize }, { min.x + gridSize, max.y - gridSize * 2.0f }, lineColor, lineSize); // Vertical left
    drawList->AddLine({ min.x + gridSize, max.y - gridSize }, { max.x - gridSize, max.y - gridSize }, lineColor, lineSize); // Horizontal
    drawList->AddLine({ max.x - gridSize, max.y - gridSize }, { max.x - gridSize, max.y - gridSize * 2.0f }, lineColor, lineSize); // Vertical right

    // The draw area
    drawList->PushClipRect({ min.x + gridSize * 2.0f, min.y + gridSize * 2.0f }, { max.x - gridSize * 2.0f + 1.0f, max.y - gridSize * 2.0f + 1.0f });
}

static std::string& GetCategory(int index)
{
    // TODO: Don't use raw numbers here!
    switch (index)
    {
        case 0: return Localise("Options_Category_System");
        case 1: return Localise("Options_Category_Input");
        case 2: return Localise("Options_Category_Audio");
        case 3: return Localise("Options_Category_Video");
    }

    return g_localeMissing;
}

static void ResetSelection()
{
    g_firstVisibleRowIndex = 0;
    g_selectedRowIndex = 0;
    g_prevSelectedRowIndex = 0;
    g_rowSelectionTime = ImGui::GetTime();
    g_prevOffsetRatio = 0.0f;
    g_leftWasHeld = false;
    g_upWasHeld = false;
    g_rightWasHeld = false;
    g_downWasHeld = false;
    g_canReset = false;
}

static bool DrawCategories()
{
    double motion = g_isStage ? 1.0 : ComputeMotion(g_appearTime, CONTAINER_CATEGORY_TIME, CONTAINER_CATEGORY_DURATION);

    if (motion == 0.0)
        return false;

    auto inputState = SWA::CInputState::GetInstance();

    bool moveLeft = !g_lockedOnOption && inputState->GetPadState().IsTapped(SWA::eKeyState_LeftBumper);
    bool moveRight = !g_lockedOnOption && inputState->GetPadState().IsTapped(SWA::eKeyState_RightBumper);

    if (moveLeft)
    {
        --g_categoryIndex;
        if (g_categoryIndex < 0)
            g_categoryIndex = g_categoryCount - 1;
    }
    else if (moveRight)
    {
        ++g_categoryIndex;
        if (g_categoryIndex >= g_categoryCount)
            g_categoryIndex = 0;
    }

    if (moveLeft || moveRight)
    {
        ResetSelection();
        Game_PlaySound("sys_actstg_score");
    }

    auto drawList = ImGui::GetBackgroundDrawList();
    auto clipRectMin = drawList->GetClipRectMin();
    auto clipRectMax = drawList->GetClipRectMax();

    float gridSize = Scale(GRID_SIZE);

    float size = Scale(32.0f);
    ImVec2 textSizes[g_categoryCount];
    float clipRectWidth = clipRectMax.x - clipRectMin.x;

    float textWidthSum = 0.0f;
    for (size_t i = 0; i < g_categoryCount; i++)
    {
        textSizes[i] = g_dfsogeistdFont->CalcTextSizeA(size, FLT_MAX, 0.0f, GetCategory(i).c_str());
        textWidthSum += textSizes[i].x;
    }

    float textSquashRatio = 1.0f;
    float maxTextWidthSum = clipRectWidth - (gridSize * 4.0f * (g_categoryCount - 1));
    if (textWidthSum > maxTextWidthSum)
    {
        textSquashRatio = maxTextWidthSum / textWidthSum;
        for (auto& textSize : textSizes)
            textSize.x *= textSquashRatio;

        textWidthSum = maxTextWidthSum;
    }

    float tabHeight = gridSize * 4.0f;
    float textPadding = (clipRectWidth - textWidthSum) / (g_categoryCount + 1.0f);
    float xOffset = textPadding;
    xOffset -= (1.0 - motion) * gridSize * 4.0;

    ImVec2 textPositions[g_categoryCount];

    for (size_t i = 0; i < g_categoryCount; i++)
    {
        float tabPadding = std::min(textPadding / 2.0f, gridSize * 3.0f);

        ImVec2 min = { clipRectMin.x + xOffset - tabPadding, clipRectMin.y };
        ImVec2 max = { min.x + textSizes[i].x + tabPadding * 2.0f, min.y + tabHeight};

        if (g_categoryIndex == i)
        {
            // Animation interrupted by entering/exiting or resizing the options menu
            if (motion < 1.0 || abs(g_categoryAnimMin.y - min.y) > 0.01f || abs(g_categoryAnimMax.y - max.y) > 0.01f)
            {
                g_categoryAnimMin = min;
                g_categoryAnimMax = max;
            }
            else
            {
                float animWidth = g_categoryAnimMax.x - g_categoryAnimMin.x;
                float width = max.x - min.x;
                float height = max.y - min.y;
                
                animWidth = Lerp(animWidth, width, 1.0f - exp(-64.0f * ImGui::GetIO().DeltaTime));

                auto center = Lerp(min, max, 0.5f);
                auto animCenter = Lerp(g_categoryAnimMin, g_categoryAnimMax, 0.5f);
                auto animatedCenter = Lerp(animCenter, center, 1.0f - exp(-16.0f * ImGui::GetIO().DeltaTime));

                float widthHalfExtent = width / 2.0f;
                float heightHalfExtent = height / 2.0f;

                g_categoryAnimMin = { animatedCenter.x - widthHalfExtent, animatedCenter.y - heightHalfExtent };
                g_categoryAnimMax = { animatedCenter.x + widthHalfExtent, animatedCenter.y + heightHalfExtent };
            }

            SetShaderModifier(IMGUI_SHADER_MODIFIER_SCANLINE_BUTTON);

            drawList->AddRectFilledMultiColor
            (
                g_categoryAnimMin,
                g_categoryAnimMax,
                IM_COL32(0, 130, 0, 223 * motion),
                IM_COL32(0, 130, 0, 178 * motion), 
                IM_COL32(0, 130, 0, 223 * motion),
                IM_COL32(0, 130, 0, 178 * motion)
            );

            drawList->AddRectFilledMultiColor
            (
                g_categoryAnimMin, 
                g_categoryAnimMax,
                IM_COL32(0, 0, 0, 13 * motion),
                IM_COL32(0, 0, 0, 0),
                IM_COL32(0, 0, 0, 55 * motion),
                IM_COL32(0, 0, 0, 6)
            );

            drawList->AddRectFilledMultiColor
            (
                g_categoryAnimMin, 
                g_categoryAnimMax,
                IM_COL32(0, 130, 0, 13 * motion),
                IM_COL32(0, 130, 0, 111 * motion),
                IM_COL32(0, 130, 0, 0), 
                IM_COL32(0, 130, 0, 55 * motion)
            );

            SetShaderModifier(IMGUI_SHADER_MODIFIER_NONE);
        }

        // Store to draw again later, otherwise the tab background gets drawn on top of text during the animation.
        textPositions[i] = { clipRectMin.x + xOffset, clipRectMin.y };
        xOffset += textSizes[i].x + textPadding;
    }

    SetScale({ textSquashRatio, 1.0f });

    for (size_t i = 0; i < g_categoryCount; i++)
    {
        auto& pos = textPositions[i];
        uint8_t alpha = (i == g_categoryIndex ? 235 : 128) * motion;
        
        SetOrigin({ pos.x, pos.y });
        SetGradient
        (
            pos,
            { pos.x + textSizes[i].x, pos.y + textSizes[i].y },
            IM_COL32(128, 255, 0, alpha),
            IM_COL32(255, 192, 0, alpha)
        );

        DrawTextWithOutline
        (
            g_dfsogeistdFont,
            size,
            pos,
            IM_COL32_WHITE,
            GetCategory(i).c_str(),
            4,
            IM_COL32_BLACK,
            IMGUI_SHADER_MODIFIER_CATEGORY_BEVEL
        );
    }

    SetScale({ 1.0f, 1.0f });
    SetOrigin({ 0.0f, 0.0f });
    ResetGradient();

    if (g_isStage || (ImGui::GetTime() - g_appearTime) >= (CONTAINER_FULL_DURATION / 60.0))
    {
        drawList->PushClipRect({ clipRectMin.x, clipRectMin.y + gridSize * 6.0f }, { clipRectMax.x - gridSize, clipRectMax.y - gridSize });
        return true;
    }

    return false;
}

static void DrawSelectionArrows(ImVec2 min, ImVec2 max, bool isLeftTapped, bool isRightTapped, bool isSlider)
{
    static constexpr double sizeMotionDuration = 16.0;

    static bool isLeftArrowMotion = false;
    static bool isRightArrowMotion = false;

    auto drawList = ImGui::GetBackgroundDrawList();
    auto gridSize = Scale(GRID_SIZE);
    auto width = gridSize * 2.5f;
    auto padding = gridSize;

    auto bgMotion = (isLeftArrowMotion || isRightArrowMotion)
        ? ComputeMotion(g_lastTappedTime, 8.0, sizeMotionDuration)
        : 0;

    if (isLeftTapped)
    {
        isLeftArrowMotion = true;
        isRightArrowMotion = false;
    }

    if (isRightTapped)
    {
        isLeftArrowMotion = false;
        isRightArrowMotion = true;
    }

    if (bgMotion >= 1.0 || isSlider)
    {
        isLeftArrowMotion = false;
        isRightArrowMotion = false;
    }

    auto getBgColour = [&](bool isAnim) -> ImU32
    {
        return IM_COL32(0, 97, 0, Lerp(96, 255, isAnim ? bgMotion : 1));
    };

    auto bgLeftColour = getBgColour(isLeftArrowMotion);
    auto bgRightColour = getBgColour(isRightArrowMotion);

    auto invertMotion = isSlider ? ComputeMotion(g_lockedOnTime, 0, VALUE_SLIDER_INTRO_DURATION) : 0;
    auto invertMotionX = invertMotion > 0.5 ? 1.0 : 0.0; // Arrow side point invert animation

    auto xAdd = Hermite(0, Scale(10), sin(invertMotion * M_PI)); // Arrow jump animation
    auto y = (min.y + max.y) / 2.0f;

    // Left triangle vertices
    auto leftX = Lerp(min.x - padding, min.x - padding - width, invertMotionX) - xAdd;
    auto leftV1Y = Hermite(min.y, max.y, invertMotion);
    auto leftV2Y = Hermite(max.y, min.y, invertMotion);
    auto leftV3X = Hermite(min.x - padding - width, min.x - padding, invertMotionX) - xAdd;

    // Right triangle vertices
    auto rightX = Lerp(max.x + padding, max.x + padding + width, invertMotionX) + xAdd;
    auto rightV1Y = Hermite(max.y, min.y, invertMotion);
    auto rightV2Y = Hermite(min.y, max.y, invertMotion);
    auto rightV3X = Hermite(max.x + padding + width, max.x + padding, invertMotionX) + xAdd;

    auto drawLeftArrow = [&](ImU32 col)
    {
        drawList->AddTriangleFilled({ leftX, leftV1Y }, { leftX, leftV2Y }, { leftV3X, y }, col);
    };

    auto drawRightArrow = [&](ImU32 col)
    {
        drawList->AddTriangleFilled({ rightX, rightV1Y }, { rightX, rightV2Y }, { rightV3X, y }, col);
    };

    drawLeftArrow(bgLeftColour);
    drawRightArrow(bgRightColour);

    // Additive gradient colours
    auto c0 = IM_COL32(255, 0, 255, 255);
    auto c1 = IM_COL32(255, 128, 255, 255);

    SetAdditive(true);

    // Apply additive gradients
    SetGradient({ leftX, leftV1Y }, { leftV3X, y }, c0, c1, c1, c0);
    drawLeftArrow(bgLeftColour);
    SetGradient({ rightX, rightV1Y }, { rightV3X, y }, c0, c1, c1, c0);
    drawRightArrow(bgRightColour);
    ResetGradient();

    if (isSlider)
    {
        auto col = IM_COL32(0, 97, 0, 255 * invertMotion);

        drawLeftArrow(col);
        drawRightArrow(col);
    }
    else
    {
        auto fgMotion = ComputeMotion(g_lastTappedTime, 0, sizeMotionDuration);
        auto fgMotionSine = sin(fgMotion * M_PI);
        auto fgScale = Lerp(0, Scale(4), fgMotionSine);
        auto fgColour = IM_COL32(0, 97, 0, 255 * fgMotionSine);

        if (isLeftArrowMotion)
        {
            drawList->AddTriangleFilled
            (
                { min.x - padding, min.y - fgScale },
                { min.x - padding, max.y + fgScale },
                { min.x - padding - width - (fgScale + Scale(2)), (min.y + max.y) / 2.0f },
                fgColour
            );
        }

        if (isRightArrowMotion)
        {
            drawList->AddTriangleFilled
            (
                { max.x + padding, max.y + fgScale },
                { max.x + padding, min.y - fgScale },
                { max.x + padding + width + (fgScale + Scale(2)), (min.y + max.y) / 2.0f },
                fgColour
            );
        }
    }

    SetAdditive(false);
}

template<typename T>
static void DrawConfigOption(int32_t rowIndex, float yOffset, ConfigDef<T>* config,
    bool isAccessible, std::string* inaccessibleReason = nullptr,
    T valueMin = T(0), T valueCenter = T(0.5), T valueMax = T(1), bool isSlider = true)
{
    auto drawList = ImGui::GetBackgroundDrawList();
    auto clipRectMin = drawList->GetClipRectMin();
    auto clipRectMax = drawList->GetClipRectMax();
    auto& padState = SWA::CInputState::GetInstance()->GetPadState();

    constexpr float OPTION_NARROW_GRID_COUNT = 36.0f;
    constexpr float OPTION_WIDE_GRID_COUNT = 54.0f;
    constexpr bool IS_SLIDER_TYPE = std::is_same_v<T, float> || std::is_same_v<T, int32_t>;

    auto isValueSlider = IS_SLIDER_TYPE && isSlider;
    auto gridSize = Scale(GRID_SIZE);
    auto optionWidth = gridSize * floor(Lerp(OPTION_NARROW_GRID_COUNT, OPTION_WIDE_GRID_COUNT, g_aspectRatioNarrowScale));
    auto optionHeight = gridSize * 5.5f;
    auto optionPadding = gridSize * 0.5f;
    auto valueWidth = Scale(192.0f);
    auto valueHeight = gridSize * 3.0f;

    // Left side
    ImVec2 min = { clipRectMin.x, clipRectMin.y + (optionHeight + optionPadding) * rowIndex + yOffset };
    ImVec2 max = { min.x + optionWidth, min.y + optionHeight };

    auto configName = config->GetNameLocalised(Config::Language);
    auto size = Scale(26.0f);
    auto textSize = g_seuratFont->CalcTextSizeA(size, FLT_MAX, 0.0f, configName.c_str());

    ImVec2 textPos = { min.x + gridSize, min.y + (optionHeight - textSize.y) / 2.0f };
    ImVec4 textClipRect = { min.x, min.y, max.x, max.y };

    bool lockedOnOption = false;

    if (g_selectedRowIndex == rowIndex)
    {
        g_selectedItem = config;
        g_inaccessibleReason = isAccessible ? nullptr : inaccessibleReason;

        if (!g_isEnterKeyBuffered)
        {
            if (isAccessible)
            {
                if constexpr (std::is_same_v<T, bool>)
                {
                    if (padState.IsTapped(SWA::eKeyState_A))
                    {
                        config->Value = !config->Value;

                        if (config->Callback)
                            config->Callback(config);

                        VideoConfigValueChangedCallback(config);
                        XAudioConfigValueChangedCallback(config);

                        Game_PlaySound("sys_worldmap_finaldecide");
                    }
                }
                else
                {
                    static T s_oldValue;

                    if (padState.IsTapped(SWA::eKeyState_A))
                    {
                        g_lockedOnOption ^= true;

                        if (g_lockedOnOption)
                        {
                            g_lockedOnTime = ImGui::GetTime();
                            g_leftWasHeld = false;
                            g_rightWasHeld = false;

                            // remember value
                            s_oldValue = config->Value;

                            if (config->LockCallback)
                                config->LockCallback(config);

                            Game_PlaySound("sys_worldmap_decide");
                        }
                        else
                        {
                            // released lock, do callbacks if value is different
                            if (config->Value != s_oldValue)
                            {
                                VideoConfigValueChangedCallback(config);
                                XAudioConfigValueChangedCallback(config);

                                if (config->ApplyCallback)
                                    config->ApplyCallback(config);
                            }

                            Game_PlaySound("sys_worldmap_finaldecide");
                        }
                    }
                    else if (padState.IsTapped(SWA::eKeyState_B))
                    {
                        // released lock, restore old value
                        config->Value = s_oldValue;

                        g_lockedOnOption = false;

                        Game_PlaySound("sys_worldmap_cansel");
                    }

                    lockedOnOption = g_lockedOnOption;
                }

                if (g_canReset && padState.IsTapped(SWA::eKeyState_X))
                {
                    if (!config->IsDefaultValue())
                    {
                        config->MakeDefault();

                        VideoConfigValueChangedCallback(config);
                        XAudioConfigValueChangedCallback(config);

                        if (config->Callback)
                            config->Callback(config);

                        if (config->ApplyCallback)
                            config->ApplyCallback(config);
                    }

                    Game_PlaySound("sys_worldmap_decide");
                }
            }
            else
            {
                if (padState.IsTapped(SWA::eKeyState_A))
                    Game_PlaySound("sys_actstg_stateserror");
            }
        }
    }

    auto fadedOut = (g_lockedOnOption && g_selectedItem != config) || !isAccessible;
    auto alpha = fadedOut ? 0.5f : 1.0f;
    auto textColour = IM_COL32(255, 255, 255, 255 * alpha);

    if (Config::Language == ELanguage::Japanese)
        textPos.y += Scale(10.0f);

    if (g_selectedItem == config)
    {
        float prevItemOffset = (g_prevSelectedRowIndex - g_selectedRowIndex) * (optionHeight + optionPadding);
        double animRatio = std::clamp((ImGui::GetTime() - g_rowSelectionTime) * 60.0 / 8.0, 0.0, 1.0);
        prevItemOffset *= pow(1.0 - animRatio, 3.0);

        auto c0 = IM_COL32(0xE2, 0x71, 0x22, isAccessible ? 0x80 : 0x30);
        auto c1 = IM_COL32(0x92, 0xFF, 0x31, isAccessible ? 0x80 : 0x30);

        drawList->AddRectFilledMultiColor({ min.x, min.y + prevItemOffset }, { max.x, max.y + prevItemOffset }, c0, c0, c1, c1);

        DrawTextWithMarquee(g_seuratFont, size, textPos, min, max, textColour, configName.c_str(), g_rowSelectionTime, 0.9, Scale(250.0));

        // large
        g_canReset = g_isControlsVisible &&
            !g_lockedOnOption &&
            g_selectedItem->GetName().find("Language") == std::string::npos &&
            g_selectedItem != &Config::WindowSize &&
            isAccessible;
    }
    else
    {
        drawList->PushClipRect(min, max, true);

        DrawRubyAnnotatedText
        (
            g_seuratFont,
            size,
            FLT_MAX,
            textPos,
            0.0f,
            configName.c_str(),
            [=](const char* str, ImVec2 pos)
            {
                DrawTextBasic(g_seuratFont, size, pos, textColour, str);
            },
            [=](const char* str, float annotationSize, ImVec2 pos)
            {
                DrawTextBasic(g_seuratFont, annotationSize, pos, textColour, str);
            }
        );

        drawList->PopClipRect();
    }

    // Right side
    min = { max.x + (clipRectMax.x - max.x - valueWidth) / 2.0f, min.y + (optionHeight - valueHeight) / 2.0f };
    max = { min.x + valueWidth, min.y + valueHeight };

    SetShaderModifier(IMGUI_SHADER_MODIFIER_SCANLINE_BUTTON);

    drawList->AddRectFilledMultiColor(min, max, IM_COL32(0, 130, 0, 223 * alpha), IM_COL32(0, 130, 0, 178 * alpha), IM_COL32(0, 130, 0, 223 * alpha), IM_COL32(0, 130, 0, 178 * alpha));
    drawList->AddRectFilledMultiColor(min, max, IM_COL32(0, 0, 0, 13 * alpha), IM_COL32(0, 0, 0, 0), IM_COL32(0, 0, 0, 55 * alpha), IM_COL32(0, 0, 0, 6 * alpha));
    drawList->AddRectFilledMultiColor(min, max, IM_COL32(0, 130, 0, 13 * alpha), IM_COL32(0, 130, 0, 111 * alpha), IM_COL32(0, 130, 0, 0), IM_COL32(0, 130, 0, 55 * alpha));

    if (isSlider)
    {
        if constexpr (std::is_same_v<T, float> || std::is_same_v<T, int32_t>)
        {
            if (lockedOnOption)
            {
                SetAdditive(true);
                drawList->AddRectFilled(min, max, IM_COL32(192, 192, 0, 96 * ComputeMotion(g_lockedOnTime, 0, VALUE_SLIDER_INTRO_DURATION)));
                SetAdditive(false);
            }

            // Inner container of slider
            const uint32_t innerColor0 = IM_COL32(0, 65, 0, 255 * alpha);
            const uint32_t innerColor1 = IM_COL32(0, 32, 0, 255 * alpha);

            float xPadding = Scale(6);
            float yPadding = Scale(3);

            drawList->AddRectFilledMultiColor
            (
                { min.x + xPadding, min.y + yPadding },
                { max.x - xPadding, max.y - yPadding },
                innerColor0,
                innerColor0,
                innerColor1,
                innerColor1
            );

            // The actual slider
            const uint32_t sliderColor0 = IM_COL32(57, 241, 0, 255 * alpha);
            const uint32_t sliderColor1 = IM_COL32(2, 106, 0, 255 * alpha);

            xPadding += Scale(2);
            yPadding += Scale(2);

            ImVec2 sliderMin = { min.x + xPadding, min.y + yPadding };
            ImVec2 sliderMax = { max.x - xPadding, max.y - yPadding };
            float factor;

            if (config->Value <= valueCenter)
                factor = float(config->Value - valueMin) / (valueCenter - valueMin) * 0.5f;
            else
                factor = 0.5f + float(config->Value - valueCenter) / (valueMax - valueCenter) * 0.5f;

            sliderMax.x = sliderMin.x + (sliderMax.x - sliderMin.x) * factor;

            drawList->AddRectFilledMultiColor(sliderMin, sliderMax, sliderColor0, sliderColor0, sliderColor1, sliderColor1);
        }
    }

    SetShaderModifier(IMGUI_SHADER_MODIFIER_NONE);

    if constexpr (std::is_same_v<T, bool>)
        DrawToggleLight({ min.x + Scale(14), min.y + ((max.y - min.y) - Scale(14)) / 2 + Scale(1) }, config->Value, alpha);

    // Selection triangles
    if (lockedOnOption)
    {
        bool leftIsHeld = padState.IsDown(SWA::eKeyState_DpadLeft) || padState.LeftStickHorizontal < -0.5f;
        bool rightIsHeld = padState.IsDown(SWA::eKeyState_DpadRight) || padState.LeftStickHorizontal > 0.5f;

        bool leftTapped = !g_leftWasHeld && leftIsHeld;
        bool rightTapped = !g_rightWasHeld && rightIsHeld;

        double time = ImGui::GetTime();

        if (leftTapped || rightTapped)
            g_lastTappedTime = time;

        bool decrement = leftTapped;
        bool increment = rightTapped;

        g_leftWasHeld = leftIsHeld;
        g_rightWasHeld = rightIsHeld;

        DrawSelectionArrows(min, max, leftTapped, rightTapped, isValueSlider);

        if constexpr (std::is_enum_v<T>)
        {
            auto it = config->EnumTemplateReverse.find(config->Value);

            if (leftTapped)
            {
                if (it == config->EnumTemplateReverse.begin())
                    it = config->EnumTemplateReverse.end();

                --it;
            }
            else if (rightTapped)
            {
                ++it;

                if (it == config->EnumTemplateReverse.end())
                    it = config->EnumTemplateReverse.begin();
            }

            config->Value = it->first;

            if (increment || decrement)
                Game_PlaySound("sys_actstg_pausecursor");
        }
        else if constexpr (std::is_same_v<T, float> || std::is_same_v<T, int32_t>)
        {
            float deltaTime = ImGui::GetIO().DeltaTime;

            bool fastIncrement = (time - g_lastTappedTime) > 0.5;
            bool isPlayIncrementSound = true;

            constexpr double INCREMENT_TIME = 1.0 / 120.0;
            constexpr double INCREMENT_SOUND_TIME = 1.0 / 7.5;

            if (isSlider)
            {
                if (fastIncrement)
                {
                    isPlayIncrementSound = (time - g_lastIncrementSoundTime) > INCREMENT_SOUND_TIME;

                    if ((time - g_lastIncrementTime) < INCREMENT_TIME)
                        fastIncrement = false;
                    else
                        g_lastIncrementTime = time;
                }

                if (fastIncrement)
                {
                    decrement = leftIsHeld;
                    increment = rightIsHeld;
                }
            }

            do
            {
                if constexpr (std::is_integral_v<T>)
                {
                    if (decrement)
                        config->Value -= 1;
                    else if (increment)
                        config->Value += 1;
                }
                else
                {
                    if (decrement)
                        config->Value -= 0.01f;
                    else if (increment)
                        config->Value += 0.01f;
                }

                deltaTime -= INCREMENT_TIME;
            }
            while (fastIncrement && deltaTime > 0.0f);

            bool isConfigValueInBounds = config->Value >= valueMin && config->Value <= valueMax;

            if ((increment || decrement) && isConfigValueInBounds && isPlayIncrementSound) 
            {
                g_lastIncrementSoundTime = time;
                Game_PlaySound("sys_actstg_twn_speechbutton");
            }

            config->Value = std::clamp(config->Value, valueMin, valueMax);
        }

        if (!config->ApplyCallback)
        {
            if ((increment || decrement) && config->Callback)
                config->Callback(config);
        }
    }

    std::string valueText;
    if constexpr (std::is_same_v<T, float>)
    {
        valueText = fmt::format("{}%", int32_t(round(config->Value * 100.0f)));
    }
    else if constexpr (std::is_same_v<T, int32_t>)
    {
        if (config == &Config::WindowSize)
        {
            if (Config::Fullscreen)
            {
                int displayW, displayH;
                GameWindow::GetSizeInPixels(&displayW, &displayH);
                valueText = fmt::format("{}x{}", displayW, displayH);
            }
            else
            {
                auto displayModes = GameWindow::GetDisplayModes();
                if (config->Value >= 0 && config->Value < displayModes.size())
                {
                    auto& displayMode = displayModes[config->Value];

                    valueText = fmt::format("{}x{}", displayMode.w, displayMode.h);
                }
                else
                {
                    valueText = fmt::format("{}x{}", GameWindow::s_width, GameWindow::s_height);
                }
            }
        }
        else if (config == &Config::Monitor)
        {
            valueText = fmt::format("{}", config->Value + 1);
        }
        else
        {
            valueText = fmt::format("{}", config->Value);

            if (isSlider && config->Value >= valueMax)
                valueText = Localise("Options_Value_Max");
        }
    }
    else
    {
        valueText = config->GetValueLocalised(Config::Language);
    }

    size = Scale(20.0f);
    textSize = g_newRodinFont->CalcTextSizeA(size, FLT_MAX, 0.0f, valueText.data());

    auto textSquashRatio = 1.0f;

    if (textSize.x > max.x - min.x)
        textSquashRatio = (max.x - min.x) / textSize.x - 0.1f;

    auto textX = min.x + ((max.x - min.x) - (textSize.x * textSquashRatio)) / 2.0f;
    auto textY = min.y + ((max.y - min.y) - textSize.y) / 2.0f;

    SetGradient
    (
        { textX, textY },
        { textX + textSize.x, textY + textSize.y },
        IM_COL32(192, 255, 0, 255),
        IM_COL32(128, 170, 0, 255)
    );

    SetScale({ textSquashRatio, 1.0f });
    SetOrigin({ textX, textY });

    DrawTextWithOutline
    (
        g_newRodinFont,
        size,
        { textX, textY },
        IM_COL32(255, 255, 255, 255 * alpha),
        valueText.data(),
        4,
        IM_COL32(0, 0, 0, 255 * alpha)
    );

    SetScale({ 1.0f, 1.0f });
    SetOrigin({ 0.0f, 0.0f });

    ResetGradient();
}

static void DrawConfigOptions()
{
    auto drawList = ImGui::GetBackgroundDrawList();
    auto clipRectMin = drawList->GetClipRectMin();
    auto clipRectMax = drawList->GetClipRectMax();

    drawList->PushClipRect({ clipRectMin.x, clipRectMin.y }, { clipRectMax.x, clipRectMax.y - Scale(5.0f) });

    g_selectedItem = nullptr;

    float gridSize = Scale(GRID_SIZE);
    float optionHeightWithPadding = gridSize * 6.0f;
    float yOffset = -g_firstVisibleRowIndex * optionHeightWithPadding;

    int32_t rowCount = 0;

    bool isStage = OptionsMenu::s_pauseMenuType == SWA::eMenuType_Stage || OptionsMenu::s_pauseMenuType == SWA::eMenuType_Hub;
    auto cmnReason = &Localise("Options_Desc_NotAvailable");

    // TODO: Don't use raw numbers here!
    switch (g_categoryIndex)
    {
        case 0: // SYSTEM
            DrawConfigOption(rowCount++, yOffset, &Config::Language, !OptionsMenu::s_isPause, cmnReason);
            DrawConfigOption(rowCount++, yOffset, &Config::VoiceLanguage, OptionsMenu::s_pauseMenuType == SWA::eMenuType_WorldMap, cmnReason);
            DrawConfigOption(rowCount++, yOffset, &Config::Subtitles, true);
            DrawConfigOption(rowCount++, yOffset, &Config::Hints, !isStage, cmnReason);
            DrawConfigOption(rowCount++, yOffset, &Config::ControlTutorial, !isStage, cmnReason);
            DrawConfigOption(rowCount++, yOffset, &Config::AchievementNotifications, true);
            DrawConfigOption(rowCount++, yOffset, &Config::TimeOfDayTransition, !Config::UseArrowsForTimeOfDayTransition);
            break;

        case 1: // INPUT
            DrawConfigOption(rowCount++, yOffset, &Config::HorizontalCamera, true);
            DrawConfigOption(rowCount++, yOffset, &Config::VerticalCamera, true);
            DrawConfigOption(rowCount++, yOffset, &Config::Vibration, true);
            DrawConfigOption(rowCount++, yOffset, &Config::AllowBackgroundInput, true);
            DrawConfigOption(rowCount++, yOffset, &Config::ControllerIcons, true);
            break;

        case 2: // AUDIO
            DrawConfigOption(rowCount++, yOffset, &Config::MasterVolume, true);
            DrawConfigOption(rowCount++, yOffset, &Config::MusicVolume, true);
            DrawConfigOption(rowCount++, yOffset, &Config::EffectsVolume, true);
            DrawConfigOption(rowCount++, yOffset, &Config::ChannelConfiguration, true);
            DrawConfigOption(rowCount++, yOffset, &Config::MusicAttenuation, AudioPatches::CanAttenuate(), &Localise("Options_Desc_OSNotSupported"));
            DrawConfigOption(rowCount++, yOffset, &Config::BattleTheme, true);
            break;

        case 3: // VIDEO
        {
            DrawConfigOption(rowCount++, yOffset, &Config::WindowSize,
                !Config::Fullscreen, &Localise("Options_Desc_NotAvailableFullscreen"),
                0, 0, (int32_t)GameWindow::GetDisplayModes().size() - 1, false);

            auto displayCount = GameWindow::GetDisplayCount();
            auto canChangeMonitor = Config::Fullscreen && displayCount > 1;
            auto monitorReason = &Localise("Options_Desc_NotAvailableWindowed");

            if (Config::Fullscreen && displayCount <= 1)
                monitorReason = &Localise("Options_Desc_NotAvailableHardware");

            DrawConfigOption(rowCount++, yOffset, &Config::Monitor, canChangeMonitor, monitorReason, 0, 0, displayCount - 1, false);

            DrawConfigOption(rowCount++, yOffset, &Config::AspectRatio, true);
            DrawConfigOption(rowCount++, yOffset, &Config::ResolutionScale, true, nullptr, 0.25f, 1.0f, 2.0f);
            DrawConfigOption(rowCount++, yOffset, &Config::Fullscreen, true);
            DrawConfigOption(rowCount++, yOffset, &Config::VSync, true);
            DrawConfigOption(rowCount++, yOffset, &Config::FPS, true, nullptr, FPS_MIN, 120, FPS_MAX);
            DrawConfigOption(rowCount++, yOffset, &Config::Brightness, true);
            DrawConfigOption(rowCount++, yOffset, &Config::AntiAliasing, true);
            DrawConfigOption(rowCount++, yOffset, &Config::TransparencyAntiAliasing, Config::AntiAliasing != EAntiAliasing::None, &Localise("Options_Desc_NotAvailableMSAA"));
            DrawConfigOption(rowCount++, yOffset, &Config::ShadowResolution, true);
            DrawConfigOption(rowCount++, yOffset, &Config::GITextureFiltering, true);
            DrawConfigOption(rowCount++, yOffset, &Config::MotionBlur, true);
            DrawConfigOption(rowCount++, yOffset, &Config::XboxColorCorrection, true);
            DrawConfigOption(rowCount++, yOffset, &Config::CutsceneAspectRatio, true);
            DrawConfigOption(rowCount++, yOffset, &Config::UIAlignmentMode, true);

            break;
        }
    }

    auto inputState = SWA::CInputState::GetInstance();

    bool upIsHeld = !g_lockedOnOption && (inputState->GetPadState().IsDown(SWA::eKeyState_DpadUp) ||
        inputState->GetPadState().LeftStickVertical > 0.5f);

    bool downIsHeld = !g_lockedOnOption && (inputState->GetPadState().IsDown(SWA::eKeyState_DpadDown) ||
        inputState->GetPadState().LeftStickVertical < -0.5f);

    bool scrollUp = !g_upWasHeld && upIsHeld;
    bool scrollDown = !g_downWasHeld && downIsHeld;

    int32_t prevSelectedRowIndex = g_selectedRowIndex;

    auto time = ImGui::GetTime();
    auto fastScroll = (time - g_lastTappedTime) > 0.6;
    auto fastScrollSpeed = 1.0 / 3.5;
    auto fastScrollEncounteredEdge = false;
    static auto fastScrollSpeedUp = false;

    if (scrollUp || scrollDown)
        g_lastTappedTime = time;

    if (!upIsHeld && !downIsHeld)
        fastScrollSpeedUp = false;

    if (fastScrollSpeedUp)
        fastScrollSpeed /= 2;

    if (fastScroll)
    {
        if ((time - g_lastIncrementTime) < fastScrollSpeed)
        {
            fastScroll = false;
        }
        else
        {
            g_lastIncrementTime = time;

            scrollUp = upIsHeld;
            scrollDown = downIsHeld;
        }
    }

    if (scrollUp)
    {
        --g_selectedRowIndex;

        if (g_selectedRowIndex < 0)
        {
            g_selectedRowIndex = fastScroll ? 0 : rowCount - 1;
            fastScrollEncounteredEdge = fastScroll;
            fastScrollSpeedUp = false;
        }
    }
    else if (scrollDown)
    {
        ++g_selectedRowIndex;

        if (g_selectedRowIndex >= rowCount)
        {
            g_selectedRowIndex = fastScroll ? rowCount - 1 : 0;
            fastScrollEncounteredEdge = fastScroll;
            fastScrollSpeedUp = false;
        }
    }

    if ((scrollUp || scrollDown) && !fastScrollEncounteredEdge)
    {
        g_rowSelectionTime = time;
        g_prevSelectedRowIndex = prevSelectedRowIndex;
        Game_PlaySound("sys_worldmap_cursor");
    }

    g_upWasHeld = upIsHeld;
    g_downWasHeld = downIsHeld;

    int32_t visibleRowCount = int32_t(floor((clipRectMax.y - clipRectMin.y) / optionHeightWithPadding));

    bool disableMoveAnimation = false;

    if (g_firstVisibleRowIndex > g_selectedRowIndex)
    {
        g_firstVisibleRowIndex = g_selectedRowIndex;
        disableMoveAnimation = true;

        if (g_selectedRowIndex > 0)
            fastScrollSpeedUp = true;
    }

    if (g_firstVisibleRowIndex + visibleRowCount - 1 < g_selectedRowIndex)
    {
        g_firstVisibleRowIndex = std::max(0, g_selectedRowIndex - visibleRowCount + 1);
        disableMoveAnimation = true;

        if (g_selectedRowIndex < rowCount - 1)
            fastScrollSpeedUp = true;
    }

    if (disableMoveAnimation)
        g_prevSelectedRowIndex = g_selectedRowIndex;

    drawList->PopClipRect();

    // Pop clip rect from DrawCategories
    drawList->PopClipRect();

    // Draw scroll bar
    if (rowCount > visibleRowCount)
    {
        float totalHeight = (clipRectMax.y - clipRectMin.y) - Scale(2.0f);
        float heightRatio = float(visibleRowCount) / float(rowCount);

        float offsetRatio = float(g_firstVisibleRowIndex) / float(rowCount);
        offsetRatio = Lerp(g_prevOffsetRatio, offsetRatio, 1.0f - exp(-16.0f * ImGui::GetIO().DeltaTime));
        g_prevOffsetRatio = offsetRatio;

        float minY = offsetRatio * totalHeight + clipRectMin.y;

        drawList->AddRectFilled
        (
            { clipRectMax.x, minY },
            { clipRectMax.x + gridSize - Scale(1.0f), minY + totalHeight * heightRatio},
            IM_COL32(0, 128, 0, 255)
        );
    }
}

static void DrawSettingsPanel(ImVec2 settingsMin, ImVec2 settingsMax)
{
    auto drawList = ImGui::GetBackgroundDrawList();

    SetProceduralOrigin(settingsMin);
    DrawContainer(settingsMin, settingsMax, true);

    if (DrawCategories())
    {
        DrawConfigOptions();

        g_isControlsVisible = true;
    }
    else
    {
        ResetSelection();
    }

    ResetProceduralOrigin();

    // Pop clip rect from DrawContainer
    drawList->PopClipRect();
}

static void DrawInfoPanel(ImVec2 infoMin, ImVec2 infoMax)
{
    auto drawList = ImGui::GetBackgroundDrawList();

    SetProceduralOrigin(infoMin);
    DrawContainer(infoMin, infoMax, false);

    auto clipRectMin = drawList->GetClipRectMin();
    auto clipRectMax = drawList->GetClipRectMax();

    if (g_selectedItem)
    {
        auto desc = g_selectedItem->GetDescription(Config::Language);
        auto thumbnail = GetThumbnail(g_selectedItem);

        float aspectRatio = float(thumbnail->width) / thumbnail->height;
        float thumbnailHeight = (clipRectMax.x - clipRectMin.x) / aspectRatio;

        ImVec2 thumbnailMin = { clipRectMin.x, clipRectMin.y + Scale(GRID_SIZE / 2.0f) };
        ImVec2 thumbnailMax = { clipRectMax.x, thumbnailMin.y + thumbnailHeight };

        if (g_isStage)
        {
            drawList->AddImage(thumbnail, thumbnailMin, thumbnailMax);
        }
        else
        {
            float time = g_appearTime + CONTAINER_FULL_DURATION / 60.0;

            drawList->AddImage(
                thumbnail,
                thumbnailMin,
                thumbnailMax,
                { 0.0f, 0.0f },
                { 1.0f, 1.0f },
                IM_COL32(255, 255, 255, 255 * TVStatic::ComputeThumbnailAlpha(time)));

            TVStatic::Draw(
                { (thumbnailMin.x + thumbnailMax.x) / 2.0f, (thumbnailMin.y + thumbnailMax.y) / 2.0f },
                { (thumbnailMax.x - thumbnailMin.x), (thumbnailMax.y - thumbnailMin.y) },
                time);
        }

        if (g_inaccessibleReason)
        {
            desc += "\n\n" + *g_inaccessibleReason;
        }
        else
        {
            // Specialised description for resolution scale.
            if (g_selectedItem == &Config::ResolutionScale)
            {
                char buf[100];
                auto resScale = round(*(float*)g_selectedItem->GetValue() * 1000) / 1000;

                std::snprintf(buf, sizeof(buf), desc.c_str(),
                    (int)((float)Video::s_viewportWidth * resScale),
                    (int)((float)Video::s_viewportHeight * resScale));

                desc = buf;
            }

            const auto& valueDescription = g_selectedItem->GetValueDescription(Config::Language);

            if (!valueDescription.empty())
                desc += "\n\n" + valueDescription;
        }

        clipRectMin = { clipRectMin.x, thumbnailMax.y };

        auto fontSize = Scale(28.0f);
        auto annotationFontSize = fontSize * ANNOTATION_FONT_SIZE_MODIFIER;

        /* Extra padding between the start
           of the description text and the
           bottom of the thumbnail. */
        auto offsetY = Scale(24.0f);

        auto textX = clipRectMin.x - Scale(0.5f);
        auto textY = thumbnailMax.y + offsetY;
        float lineWidth = clipRectMax.x - clipRectMin.x;

        if (Config::Language == ELanguage::Japanese)
        {
            /* Removing some padding of the applied due
               to the inclusion of annotation for Japanese. */
            textY -= Scale(8.0f);

            /* The annotation (and thus the Japanese) can be
               drawn above the edges of the info panel thus the
               clip needs to be extended a bit. */
            clipRectMin.x -= annotationFontSize;
            clipRectMax.x += annotationFontSize;

            textY += annotationFontSize;
            
            // Dirty hack to disallow clipping on Japanese text
            // whilst allowing annotations to go over the border
            lineWidth -= annotationFontSize;
        }

        auto textSize = MeasureCentredParagraph(g_seuratFont, fontSize, lineWidth, 5.0f, desc.c_str());

        drawList->PushClipRect(clipRectMin, clipRectMax, false);

        static auto isScrolling = false;
        static auto isManualScrolling = false;
        static auto scrollOffset = 0.0f;
        static auto scrollTimer = 0.0f;
        static auto scrollDirection = 1.0f;
        auto scrollMax = textSize.y - (clipRectMax.y - textY);
        auto scrollSpeed = Scale(50);

        if (scrollMax > 0.0f)
        {
            auto vert = -g_rightStickY;
        
            if (fabs(vert) > 0.25f)
            {
                isManualScrolling = true;
                scrollOffset += vert * scrollSpeed * App::s_deltaTime;
            }
            else if (isManualScrolling && fabs(vert) <= 0.25f)
            {
                isScrolling = false;
                isManualScrolling = false;
                scrollTimer = 0.0f;
                scrollDirection = vert > 0.0f ? 1.0f : -1.0f;
            }
        
            if (!isManualScrolling)
            {
                if (!isScrolling)
                {
                    scrollTimer += App::s_deltaTime;
            
                    if (scrollTimer >= INFO_TEXT_MARQUEE_DELAY)
                        isScrolling = true;
                }
            
                if (isScrolling)
                {
                    scrollOffset += scrollSpeed * scrollDirection * App::s_deltaTime;
            
                    if (scrollOffset >= scrollMax)
                    {
                        isScrolling = false;
                        scrollOffset = scrollMax;
                        scrollTimer = 0.0f;
                        scrollDirection = -1.0f;
                    }
                    else if (scrollOffset <= 0.0f)
                    {
                        isScrolling = false;
                        scrollOffset = 0;
                        scrollTimer = 0.0f;
                        scrollDirection = 1.0f;
                    }
                }
            }
        
            scrollOffset = std::clamp(scrollOffset, 0.0f, scrollMax);
        }
        else
        {
            isScrolling = false;
            scrollOffset = 0.0f;
            scrollTimer = 0.0f;
            scrollDirection = 1.0f;
        }

        SetVerticalMarqueeFade({ clipRectMin.x, clipRectMin.y + Scale(5.5f) }, clipRectMax, Scale(10), Scale(10));

        DrawRubyAnnotatedText
        (
            g_seuratFont,
            fontSize,
            lineWidth,
            { textX, textY - scrollOffset },
            5.0f,
            desc.c_str(),
            [=](const char* str, ImVec2 pos)
            {
                DrawTextBasic(g_seuratFont, fontSize, pos, IM_COL32_WHITE, str);
            },
            [=](const char* str, float size, ImVec2 pos)
            {
                DrawTextBasic(g_seuratFont, size, pos, IM_COL32_WHITE, str);
            }
        );

        ResetMarqueeFade();

        drawList->PopClipRect();

        // Reset parameters on new selected row.
        if (ImGui::GetTime() - g_rowSelectionTime <= 0.0f)
        {
            isScrolling = false;
            scrollOffset = 0.0f;
            scrollTimer = 0.0f;
            scrollDirection = 1.0f;
        }
    }

    ResetProceduralOrigin();

    // Pop clip rect from DrawContainer
    drawList->PopClipRect();
}

static void SetOptionsMenuVisible(bool isVisible)
{
    OptionsMenu::s_isVisible = isVisible;
    *SWA::SGlobals::ms_IsRenderHud = !isVisible;
}

static bool DrawMilesElectric()
{
    auto drawList = ImGui::GetBackgroundDrawList();
    auto& res = ImGui::GetIO().DisplaySize;

    // Compensate for the lack of CSD UI dimming the background.
    if (g_isStage)
        drawList->AddRectFilled({ 0.0f, 0.0f }, res, IM_COL32(0, 0, 0, 127));

    auto scaleMotion = ComputeMotion(g_appearTime, 0, MILES_ELECTRIC_SCALE_DURATION);

    if (scaleMotion >= 1.0)
    {
        if (g_isClosing)
            SetOptionsMenuVisible(false);
    
        return true;
    }

    auto bgAlphaMotion = ComputeMotion(g_appearTime, 0, MILES_ELECTRIC_BACKGROUND_DURATION);
    auto bgAlpha = g_isClosing
        ? Hermite(255, 0, bgAlphaMotion)
        : Hermite(0, 255, bgAlphaMotion);

    drawList->AddRectFilled({ 0, 0 }, res, IM_COL32(64, 64, 64, bgAlpha));

    auto y = g_isClosing
        ? Hermite(140, 0, scaleMotion)
        : Hermite(0, 140, scaleMotion);

    auto scale = g_isClosing
        ? Hermite(Scale(1400), Scale(64), scaleMotion)
        : Hermite(Scale(64), Scale(1400), scaleMotion);

    ImVec2 centre = { res.x / 2, res.y / 2 - Scale(y) };

    auto alpha = g_isClosing
        ? Hermite(255, 0, scaleMotion)
        : Hermite(255, 127, scaleMotion);

    drawList->AddImage
    (
        g_upMilesElectric.get(),
        { centre.x - scale, centre.y - scale },
        { centre.x + scale, centre.y + scale },
        { 0, 0 },
        { 1, 1 },
        IM_COL32(255, 255, 255, alpha)
    );

    return false;
}

static bool DrawFadeTransition()
{
    auto drawList = ImGui::GetBackgroundDrawList();
    auto& res = ImGui::GetIO().DisplaySize;

    auto scaleMotion = ComputeMotion(g_appearTime, 0, MILES_ELECTRIC_SCALE_DURATION);
    auto fgAlphaOutMotion = ComputeMotion(g_appearTime, MILES_ELECTRIC_FOREGROUND_FADE_OUT_TIME, MILES_ELECTRIC_FOREGROUND_FADE_DURATION);

    if (scaleMotion < 0.8)
        return false;

    drawList->AddRectFilled({ 0, 0 }, res, IM_COL32(0, 0, 0, Lerp(255, 0, fgAlphaOutMotion)));

    return fgAlphaOutMotion >= 1.0;
}

void OptionsMenu::Init()
{
    auto& io = ImGui::GetIO();

    g_seuratFont = ImFontAtlasSnapshot::GetFont("FOT-SeuratPro-M.otf");
    g_dfsogeistdFont = ImFontAtlasSnapshot::GetFont("DFSoGeiStd-W7.otf");
    g_newRodinFont = ImFontAtlasSnapshot::GetFont("FOT-NewRodinPro-DB.otf");

    LoadThumbnails();

    g_upMilesElectric = LOAD_ZSTD_TEXTURE(g_miles_electric);

    TVStatic::Init();
}

void OptionsMenu::Draw()
{
    if (!s_isVisible)
        return;

    // We've entered the menu now, no need to check this.
    auto pInputState = SWA::CInputState::GetInstance();
    if (pInputState->GetPadState().IsReleased(SWA::eKeyState_A))
        g_isEnterKeyBuffered = false;

    if (g_isStage)
    {
        if (!DrawMilesElectric())
            return;
    }

    if (!g_isClosing)
    {
        auto drawList = ImGui::GetBackgroundDrawList();
        auto& res = ImGui::GetIO().DisplaySize;

        if (g_isStage)
            drawList->AddRectFilled({ 0.0f, 0.0f }, res, IM_COL32(0, 0, 0, 223));

        DrawScanlineBars();

        float settingsGridCount = floor(Lerp(SETTINGS_NARROW_GRID_COUNT, SETTINGS_WIDE_GRID_COUNT, g_aspectRatioNarrowScale));
        float paddingGridCount = Lerp(PADDING_NARROW_GRID_COUNT, PADDING_WIDE_GRID_COUNT, g_aspectRatioNarrowScale);
        float infoGridCount = floor(Lerp(INFO_NARROW_GRID_COUNT, INFO_WIDE_GRID_COUNT, g_aspectRatioNarrowScale));
        float totalGridCount = settingsGridCount + paddingGridCount + infoGridCount;

        float minX = round(g_aspectRatioOffsetX + Scale((1280.0f - (GRID_SIZE * totalGridCount)) / 2.0f));
        float maxX = res.x - minX;
        float minY = round(g_aspectRatioOffsetY + Scale(CONTAINER_POS_Y));
        float maxY = round(g_aspectRatioOffsetY + Scale((720.0f - CONTAINER_POS_Y + 1.0f)));

        DrawSettingsPanel(
            { minX, minY },
            { minX + Scale(settingsGridCount * GRID_SIZE), maxY }
        );

        DrawInfoPanel(
            { maxX - Scale(infoGridCount * GRID_SIZE) - 1.0f, minY },
            { maxX - 1.0f, maxY }
        );

        if (g_isStage)
            DrawFadeTransition();
    }

    s_isRestartRequired = Config::Language != App::s_language;
}

void OptionsMenu::Open(bool isPause, SWA::EMenuType pauseMenuType)
{
    g_isClosing = false;
    s_isPause = isPause;
    s_pauseMenuType = pauseMenuType;
    g_isStage = isPause && pauseMenuType != SWA::eMenuType_WorldMap;
    
    g_appearTime = ImGui::GetTime();
    g_categoryIndex = 0;
    g_categoryAnimMin = { 0.0f, 0.0f };
    g_categoryAnimMax = { 0.0f, 0.0f };
    g_selectedItem = nullptr;
    g_titleAnimBegin = true;

    /* Store button state so we can track it later
       and prevent the first item being selected. */
    if (SWA::CInputState::GetInstance()->GetPadState().IsDown(SWA::eKeyState_A))
        g_isEnterKeyBuffered = true;

    ResetSelection();
    SetOptionsMenuVisible(true);

    std::array<Button, 4> buttons =
    {
        Button("Common_Switch", 115.0f, EButtonIcon::LBRB, EButtonAlignment::Left, &g_isControlsVisible),
        Button("Common_Reset", 110.0f, EButtonIcon::X, &g_canReset),
        Button("Common_Select", 115.0f, EButtonIcon::A, &g_isControlsVisible),
        Button("Common_Back", 65.0f, EButtonIcon::B, &g_isControlsVisible)
    };
    
    ButtonGuide::Open(buttons);
    ButtonGuide::SetSideMargins(250);

    hid::SetProhibitedInputs(XAMINPUT_GAMEPAD_START, false, true);
}

void OptionsMenu::Close()
{
    if (!g_isClosing)
    {
        g_appearTime = ImGui::GetTime();
        g_isControlsVisible = false;
        g_isClosing = true;

        ButtonGuide::Close();
        Config::Save();

        hid::SetProhibitedInputs();
    }

    // Skip Miles Electric animation at main menu.
    if (!g_isStage)
        SetOptionsMenuVisible(false);
}

bool OptionsMenu::CanClose()
{
    return !g_lockedOnOption && g_isControlsVisible;
}
