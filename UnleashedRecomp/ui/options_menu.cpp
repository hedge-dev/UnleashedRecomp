#include "options_menu.h"
#include "imgui_utils.h"
#include "window.h"
#include "exports.h"

#include <api/SWA/System/InputState.h>
#include <gpu/imgui_common.h>
#include <gpu/video.h>
#include <kernel/heap.h>
#include <kernel/memory.h>
#include <locale/locale.h>

#include <patches/audio_patches.h>

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

constexpr float COMMON_PADDING_POS_Y = 118.0f;
constexpr float COMMON_PADDING_POS_X = 30.0f;
constexpr float INFO_CONTAINER_POS_X = 870.0f;

static constexpr int32_t m_categoryCount = 4;
static int32_t m_categoryIndex;
static ImVec2 m_categoryAnimMin;
static ImVec2 m_categoryAnimMax;

static int32_t m_firstVisibleRowIndex;
static int32_t m_prevSelectedRowIndex;
static int32_t m_selectedRowIndex;
static double m_rowSelectionTime;

static bool m_leftWasHeld;
static bool m_upWasHeld;
static bool m_rightWasHeld;
static bool m_downWasHeld;

static bool m_lockedOnOption;
static double m_lastTappedTime;
static double m_lastIncrementTime;
static double m_lastIncrementSoundTime;

static constexpr size_t GRID_SIZE = 9;

static ImFont* m_seuratFont;
static ImFont* m_dfsogeistdFont;
static ImFont* m_newRodinFont;

static const IConfigDef* m_selectedItem;

static std::string* m_inaccessibleReason;

static bool m_isEnterKeyBuffered = false;

static double m_appearTime = 0.0;

static void DrawScanlineBars()
{
    constexpr uint32_t COLOR0 = IM_COL32(203, 255, 0, 0);
    constexpr uint32_t COLOR1 = IM_COL32(203, 255, 0, 55);
    constexpr uint32_t FADE_COLOR0 = IM_COL32(0, 0, 0, 255);
    constexpr uint32_t FADE_COLOR1 = IM_COL32(0, 0, 0, 0);
    constexpr uint32_t OUTLINE_COLOR = IM_COL32(115, 178, 104, 255);

    float height = Scale(105.0f);

    auto& res = ImGui::GetIO().DisplaySize;
    auto drawList = ImGui::GetForegroundDrawList();

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
    drawList->AddRectFilledMultiColor
    (
        { res.x, res.y },
        { 0.0f, res.y - height },
        COLOR0,
        COLOR0,
        COLOR1,
        COLOR1
    );

    SetShaderModifier(IMGUI_SHADER_MODIFIER_NONE);

    // Options text
    // TODO: localise this.
    DrawTextWithOutline<int>(m_dfsogeistdFont, Scale(48.0f), { Scale(122.0f), Scale(56.0f) }, IM_COL32(255, 195, 0, 255), "OPTIONS", 4, IM_COL32_BLACK);

    // Top bar line
    drawList->AddLine
    (
        { 0.0f, height },
        { res.x, height },
        OUTLINE_COLOR,
        Scale(1)
    );

    // Bottom bar line
    drawList->AddLine
    (
        { 0.0f, res.y - height },
        { res.x, res.y - height },
        OUTLINE_COLOR,
        Scale(1)
    );
}

static float AlignToNextGrid(float value)
{
    return floor(value / GRID_SIZE) * GRID_SIZE;
}

static void DrawContainer(ImVec2 min, ImVec2 max)
{
    double containerHeight = ComputeMotion(m_appearTime, 0.0, CONTAINER_LINE_ANIMATION_DURATION);

    float center = (min.y + max.y) / 2.0f;
    min.y = Lerp(center, min.y, containerHeight);
    max.y = Lerp(center, max.y, containerHeight);

    auto& res = ImGui::GetIO().DisplaySize;
    auto drawList = ImGui::GetForegroundDrawList();

    double outerAlpha = ComputeMotion(m_appearTime, CONTAINER_OUTER_TIME, CONTAINER_OUTER_DURATION);
    double innerAlpha = ComputeMotion(m_appearTime, CONTAINER_INNER_TIME, CONTAINER_INNER_DURATION);
    double backgroundAlpha = ComputeMotion(m_appearTime, CONTAINER_BACKGROUND_TIME, CONTAINER_BACKGROUND_DURATION);

    const uint32_t lineColor = IM_COL32(0, 89, 0, 255 * containerHeight);
    const uint32_t outerColor = IM_COL32(0, 49, 0, 255 * outerAlpha);
    const uint32_t innerColor = IM_COL32(0, 33, 0, 255 * innerAlpha);
    const uint32_t backgroundColor = IM_COL32(0, 0, 0, 223 * backgroundAlpha);

    float gridSize = Scale(GRID_SIZE);

    drawList->AddRectFilled(min, max, backgroundColor); // Background

    SetShaderModifier(IMGUI_SHADER_MODIFIER_CHECKERBOARD);

    drawList->AddRectFilled(min, { min.x + gridSize, max.y }, outerColor); // Container outline left
    drawList->AddRectFilled({ max.x - gridSize, min.y }, max, outerColor); // Container outline right
    drawList->AddRectFilled({ min.x + gridSize, min.y }, { max.x - gridSize, min.y + gridSize }, outerColor); // Container outline top
    drawList->AddRectFilled({ min.x + gridSize, max.y - gridSize }, { max.x - gridSize, max.y }, outerColor); // Container outline bottom

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
    m_firstVisibleRowIndex = 0;
    m_selectedRowIndex = 0;
    m_prevSelectedRowIndex = 0;
    m_rowSelectionTime = ImGui::GetTime();
    m_leftWasHeld = false;
    m_upWasHeld = false;
    m_rightWasHeld = false;
    m_downWasHeld = false;
}

static bool DrawCategories()
{
    double motion = ComputeMotion(m_appearTime, CONTAINER_CATEGORY_TIME, CONTAINER_CATEGORY_DURATION);

    if (motion == 0.0)
        return false;

    auto inputState = SWA::CInputState::GetInstance();

    bool moveLeft = !m_lockedOnOption && (inputState->GetPadState().IsTapped(SWA::eKeyState_LeftBumper) ||
        inputState->GetPadState().IsTapped(SWA::eKeyState_LeftTrigger));

    bool moveRight = !m_lockedOnOption && (inputState->GetPadState().IsTapped(SWA::eKeyState_RightBumper) ||
        inputState->GetPadState().IsTapped(SWA::eKeyState_RightTrigger));

    if (moveLeft)
    {
        --m_categoryIndex;
        if (m_categoryIndex < 0)
            m_categoryIndex = m_categoryCount - 1;
    }
    else if (moveRight)
    {
        ++m_categoryIndex;
        if (m_categoryIndex >= m_categoryCount)
            m_categoryIndex = 0;
    }

    if (moveLeft || moveRight)
    {
        ResetSelection();
        Game_PlaySound("sys_actstg_score");
    }

    auto drawList = ImGui::GetForegroundDrawList();
    auto clipRectMin = drawList->GetClipRectMin();
    auto clipRectMax = drawList->GetClipRectMax();

    float gridSize = Scale(GRID_SIZE);
    float textPadding = gridSize * 3.0f;
    float tabPadding = gridSize;

    float size = Scale(32.0f);
    ImVec2 textSizes[m_categoryCount];
    float tabWidthSum = 0.0f;
    for (size_t i = 0; i < m_categoryCount; i++)
    {
        textSizes[i] = m_dfsogeistdFont->CalcTextSizeA(size, FLT_MAX, 0.0f, GetCategory(i).c_str());
        tabWidthSum += textSizes[i].x + textPadding * 2.0f;
    }
    tabWidthSum += (m_categoryCount - 1) * tabPadding;

    float tabHeight = gridSize * 4.0f;
    float xOffset = ((clipRectMax.x - clipRectMin.x) - tabWidthSum) / 2.0f;
    xOffset -= (1.0 - motion) * gridSize * 4.0;

    ImVec2 minVec[m_categoryCount];

    for (size_t i = 0; i < m_categoryCount; i++)
    {
        ImVec2 min = { clipRectMin.x + xOffset, clipRectMin.y };

        xOffset += textSizes[i].x + textPadding * 2.0f;
        ImVec2 max = { clipRectMin.x + xOffset, clipRectMin.y + tabHeight };
        xOffset += tabPadding;

        if (m_categoryIndex == i)
        {
            // Animation interrupted by entering/exiting or resizing the options menu
            if (motion < 1.0 || abs(m_categoryAnimMin.y - min.y) > 0.01f || abs(m_categoryAnimMax.y - max.y) > 0.01f)
            {
                m_categoryAnimMin = min;
                m_categoryAnimMax = max;
            }
            else
            {
                float animWidth = m_categoryAnimMax.x - m_categoryAnimMin.x;
                float width = max.x - min.x;
                float height = max.y - min.y;
                
                animWidth = Lerp(animWidth, width, 1.0f - exp(-64.0f * ImGui::GetIO().DeltaTime));

                auto center = Lerp(min, max, 0.5f);
                auto animCenter = Lerp(m_categoryAnimMin, m_categoryAnimMax, 0.5f);
                auto animatedCenter = Lerp(animCenter, center, 1.0f - exp(-16.0f * ImGui::GetIO().DeltaTime));

                float widthHalfExtent = width / 2.0f;
                float heightHalfExtent = height / 2.0f;

                m_categoryAnimMin = { animatedCenter.x - widthHalfExtent, animatedCenter.y - heightHalfExtent };
                m_categoryAnimMax = { animatedCenter.x + widthHalfExtent, animatedCenter.y + heightHalfExtent };
            }

            SetShaderModifier(IMGUI_SHADER_MODIFIER_SCANLINE_BUTTON);

            drawList->AddRectFilledMultiColor
            (
                m_categoryAnimMin,
                m_categoryAnimMax,
                IM_COL32(0, 130, 0, 223 * motion),
                IM_COL32(0, 130, 0, 178 * motion), 
                IM_COL32(0, 130, 0, 223 * motion),
                IM_COL32(0, 130, 0, 178 * motion)
            );

            drawList->AddRectFilledMultiColor
            (
                m_categoryAnimMin, 
                m_categoryAnimMax,
                IM_COL32(0, 0, 0, 13 * motion),
                IM_COL32(0, 0, 0, 0),
                IM_COL32(0, 0, 0, 55 * motion),
                IM_COL32(0, 0, 0, 6)
            );

            drawList->AddRectFilledMultiColor
            (
                m_categoryAnimMin, 
                m_categoryAnimMax,
                IM_COL32(0, 130, 0, 13 * motion),
                IM_COL32(0, 130, 0, 111 * motion),
                IM_COL32(0, 130, 0, 0), 
                IM_COL32(0, 130, 0, 55 * motion)
            );

            SetShaderModifier(IMGUI_SHADER_MODIFIER_NONE);
        }

        min.x += textPadding;

        // Store to draw again later, otherwise the tab background gets drawn on top of text during the animation.
        minVec[i] = min; 
    }

    for (size_t i = 0; i < m_categoryCount; i++)
    {
        auto& min = minVec[i];
        uint8_t alpha = (i == m_categoryIndex ? 235 : 128) * motion;

        SetGradient
        (
            min,
            { min.x + textSizes[i].x, min.y + textSizes[i].y },
            IM_COL32(128, 255, 0, alpha),
            IM_COL32(255, 192, 0, alpha)
        );

        DrawTextWithOutline<int>
        (
            m_dfsogeistdFont,
            size,
            min,
            IM_COL32_WHITE,
            GetCategory(i).c_str(),
            3,
            IM_COL32_BLACK
        );

        ResetGradient();
    }

    if ((ImGui::GetTime() - m_appearTime) >= (CONTAINER_FULL_DURATION / 60.0))
    {
        drawList->PushClipRect({ clipRectMin.x, clipRectMin.y + gridSize * 6.0f }, { clipRectMax.x - gridSize, clipRectMax.y - gridSize });
        return true;
    }

    return false;
}

template<typename T>
static void DrawConfigOption(int32_t rowIndex, float yOffset, ConfigDef<T>* config,
    bool isAccessible, std::string* inaccessibleReason = nullptr,
    T valueMin = T(0), T valueCenter = T(0.5), T valueMax = T(1))
{
    auto drawList = ImGui::GetForegroundDrawList();
    auto clipRectMin = drawList->GetClipRectMin();
    auto clipRectMax = drawList->GetClipRectMax();
    auto& padState = SWA::CInputState::GetInstance()->GetPadState();

    auto gridSize = Scale(GRID_SIZE);
    auto optionWidth = gridSize * 54.0f;
    auto optionHeight = gridSize * 5.5f;
    auto optionPadding = gridSize * 0.5f;
    auto valueWidth = Scale(192.0f);
    auto valueHeight = gridSize * 3.0f;
    auto triangleWidth = gridSize * 2.5f;
    auto trianglePadding = gridSize;

    // Left side
    ImVec2 min = { clipRectMin.x, clipRectMin.y + (optionHeight + optionPadding) * rowIndex + yOffset };
    ImVec2 max = { min.x + optionWidth, min.y + optionHeight };

    auto configName = config->GetNameLocalised();
    auto size = Scale(26.0f);
    auto textSize = m_seuratFont->CalcTextSizeA(size, FLT_MAX, 0.0f, configName.c_str());

    ImVec2 textPos = { min.x + gridSize, min.y + (optionHeight - textSize.y) / 2.0f };
    ImVec4 textClipRect = { min.x, min.y, max.x, max.y };

    bool lockedOnOption = false;
    if (m_selectedRowIndex == rowIndex)
    {
        m_selectedItem = config;
        m_inaccessibleReason = isAccessible ? nullptr : inaccessibleReason;

        if (!m_isEnterKeyBuffered)
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

                        Game_PlaySound("sys_worldmap_finaldecide");
                    }
                }
                else
                {
                    static T s_oldValue;

                    if (padState.IsTapped(SWA::eKeyState_A))
                    {
                        m_lockedOnOption ^= true;

                        if (m_lockedOnOption)
                        {
                            m_leftWasHeld = false;
                            m_rightWasHeld = false;
                            // remember value
                            s_oldValue = config->Value;

                            Game_PlaySound("sys_worldmap_decide");
                        }
                        else
                        {
                            // released lock, call video callbacks if value is different
                            if (config->Value != s_oldValue)
                                VideoConfigValueChangedCallback(config);

                            Game_PlaySound("sys_worldmap_finaldecide");
                        }
                    }
                    else if (padState.IsTapped(SWA::eKeyState_B))
                    {
                        // released lock, restore old value
                        config->Value = s_oldValue;
                        m_lockedOnOption = false;

                        Game_PlaySound("sys_worldmap_cansel");
                    }

                    lockedOnOption = m_lockedOnOption;
                }
            }
            else
            {
                if (padState.IsTapped(SWA::eKeyState_A))
                    Game_PlaySound("sys_actstg_stateserror");
            }
        }
    }

    auto fadedOut = (m_lockedOnOption && m_selectedItem != config) || !isAccessible;
    auto alpha = fadedOut ? 0.5f : 1.0f;
    auto textColour = IM_COL32(255, 255, 255, 255 * alpha);

    if (m_selectedItem == config)
    {
        float prevItemOffset = (m_prevSelectedRowIndex - m_selectedRowIndex) * (optionHeight + optionPadding);
        double animRatio = std::clamp((ImGui::GetTime() - m_rowSelectionTime) * 60.0 / 8.0, 0.0, 1.0);
        prevItemOffset *= pow(1.0 - animRatio, 3.0);

        auto c0 = IM_COL32(0xE2, 0x71, 0x22, isAccessible ? 0x80 : 0x30);
        auto c1 = IM_COL32(0x92, 0xFF, 0x31, isAccessible ? 0x80 : 0x30);

        drawList->AddRectFilledMultiColor({ min.x, min.y + prevItemOffset }, { max.x, max.y + prevItemOffset }, c0, c0, c1, c1);

        DrawTextWithMarquee(m_seuratFont, size, textPos, min, max, textColour, configName.c_str(), m_rowSelectionTime, 0.9, 250.0);
    }
    else
    {
        drawList->AddText(m_seuratFont, size, textPos, textColour, configName.c_str(), 0, 0.0f, &textClipRect);
    }

    // Right side
    min = { max.x + (clipRectMax.x - max.x - valueWidth) / 2.0f, min.y + (optionHeight - valueHeight) / 2.0f };
    max = { min.x + valueWidth, min.y + valueHeight };

    SetShaderModifier(IMGUI_SHADER_MODIFIER_SCANLINE_BUTTON);

    drawList->AddRectFilledMultiColor(min, max, IM_COL32(0, 130, 0, 223 * alpha), IM_COL32(0, 130, 0, 178 * alpha), IM_COL32(0, 130, 0, 223 * alpha), IM_COL32(0, 130, 0, 178 * alpha));
    drawList->AddRectFilledMultiColor(min, max, IM_COL32(0, 0, 0, 13 * alpha), IM_COL32(0, 0, 0, 0), IM_COL32(0, 0, 0, 55 * alpha), IM_COL32(0, 0, 0, 6 * alpha));
    drawList->AddRectFilledMultiColor(min, max, IM_COL32(0, 130, 0, 13 * alpha), IM_COL32(0, 130, 0, 111 * alpha), IM_COL32(0, 130, 0, 0), IM_COL32(0, 130, 0, 55 * alpha));

    if constexpr (std::is_same_v<T, float> || std::is_same_v<T, int32_t>)
    {
        // Inner container of slider
        const uint32_t innerColor0 = IM_COL32(0, 65, 0, 255 * alpha);
        const uint32_t innerColor1 = IM_COL32(0, 32, 0, 255 * alpha);

        float xPadding = Scale(6.0f);
        float yPadding = Scale(3.0f);

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

        xPadding += Scale(1.0f);
        yPadding += Scale(1.0f);

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

    SetShaderModifier(IMGUI_SHADER_MODIFIER_NONE);

    // Selection triangles
    if (lockedOnOption)
    {
        constexpr uint32_t COLOR = IM_COL32(0, 97, 0, 255);

        // Left
        drawList->AddTriangleFilled
        (
            { min.x - trianglePadding, min.y },
            { min.x - trianglePadding, max.y }, 
            { min.x - trianglePadding - triangleWidth, (min.y + max.y) / 2.0f }, 
            COLOR
        );

        // Right
        drawList->AddTriangleFilled
        (
            { max.x + trianglePadding, max.y },
            { max.x + trianglePadding, min.y },
            { max.x + trianglePadding + triangleWidth, (min.y + max.y) / 2.0f },
            COLOR
        );

        bool leftIsHeld = padState.IsDown(SWA::eKeyState_DpadLeft) || padState.LeftStickHorizontal < -0.5f;
        bool rightIsHeld = padState.IsDown(SWA::eKeyState_DpadRight) || padState.LeftStickHorizontal > 0.5f;

        bool leftTapped = !m_leftWasHeld && leftIsHeld;
        bool rightTapped = !m_rightWasHeld && rightIsHeld;

        double time = ImGui::GetTime();

        if (leftTapped || rightTapped)
            m_lastTappedTime = time;

        bool decrement = leftTapped;
        bool increment = rightTapped;

        bool fastIncrement = (time - m_lastTappedTime) > 0.5;
        constexpr double INCREMENT_TIME = 1.0 / 120.0;

        constexpr double INCREMENT_SOUND_TIME = 1.0 / 7.5;
        bool isPlayIncrementSound = true;

        if (fastIncrement)
        {
            isPlayIncrementSound = (time - m_lastIncrementSoundTime) > INCREMENT_SOUND_TIME;

            if ((time - m_lastIncrementTime) < INCREMENT_TIME)
                fastIncrement = false;
            else
                m_lastIncrementTime = time;
        }

        if (fastIncrement)
        {
            decrement = leftIsHeld;
            increment = rightIsHeld;
        }

        m_leftWasHeld = leftIsHeld;
        m_rightWasHeld = rightIsHeld;

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
                m_lastIncrementSoundTime = time;
                Game_PlaySound("sys_actstg_twn_speechbutton");
            }

            config->Value = std::clamp(config->Value, valueMin, valueMax);
        }

        if (config->Callback)
            config->Callback(config);
    }

    std::string valueText;
    if constexpr (std::is_same_v<T, float>)
    {
        valueText = std::format("{}%", int32_t(round(config->Value * 100.0f)));
    }
    else if constexpr (std::is_same_v<T, int32_t>)
    {
        valueText = config->Value >= valueMax ? Localise("Options_Value_Max") : std::format("{}", config->Value);
    }
    else
    {
        valueText = config->GetValueLocalised();
    }

    size = Scale(20.0f);
    textSize = m_newRodinFont->CalcTextSizeA(size, FLT_MAX, 0.0f, valueText.data());

    min.x += ((max.x - min.x) - textSize.x) / 2.0f;
    min.y += ((max.y - min.y) - textSize.y) / 2.0f;

    SetGradient
    (
        min,
        { min.x + textSize.x, min.y + textSize.y },
        IM_COL32(192, 255, 0, 255),
        IM_COL32(128, 170, 0, 255)
    );

    DrawTextWithOutline<int>
    (
        m_newRodinFont,
        size,
        min,
        IM_COL32(255, 255, 255, 255 * alpha),
        valueText.data(),
        2,
        IM_COL32(0, 0, 0, 255 * alpha)
    );

    ResetGradient();
}

static void DrawConfigOptions()
{
    auto drawList = ImGui::GetForegroundDrawList();
    auto clipRectMin = drawList->GetClipRectMin();
    auto clipRectMax = drawList->GetClipRectMax();

    m_selectedItem = nullptr;

    float gridSize = Scale(GRID_SIZE);
    float optionHeightWithPadding = gridSize * 6.0f;
    float yOffset = -m_firstVisibleRowIndex * optionHeightWithPadding;

    int32_t rowCount = 0;

    bool isStage = OptionsMenu::s_pauseMenuType == SWA::eMenuType_Stage || OptionsMenu::s_pauseMenuType == SWA::eMenuType_Hub;
    auto cmnReason = &Localise("Options_Desc_NotAvailable");

    // TODO: Don't use raw numbers here!
    switch (m_categoryIndex)
    {
    case 0: // SYSTEM
        DrawConfigOption(rowCount++, yOffset, &Config::Language, !OptionsMenu::s_isPause, cmnReason);
        DrawConfigOption(rowCount++, yOffset, &Config::Hints, !isStage, cmnReason);
        DrawConfigOption(rowCount++, yOffset, &Config::ControlTutorial, !isStage, cmnReason);
        DrawConfigOption(rowCount++, yOffset, &Config::AchievementNotifications, true);
        DrawConfigOption(rowCount++, yOffset, &Config::SaveScoreAtCheckpoints, true);
        DrawConfigOption(rowCount++, yOffset, &Config::UnleashGaugeBehaviour, true);
        DrawConfigOption(rowCount++, yOffset, &Config::TimeOfDayTransition, true);
        DrawConfigOption(rowCount++, yOffset, &Config::SkipIntroLogos, true);
        break;
    case 1: // INPUT
        DrawConfigOption(rowCount++, yOffset, &Config::InvertCameraX, true);
        DrawConfigOption(rowCount++, yOffset, &Config::InvertCameraY, true);
        DrawConfigOption(rowCount++, yOffset, &Config::XButtonHoming, OptionsMenu::s_pauseMenuType == SWA::eMenuType_WorldMap, cmnReason);
        DrawConfigOption(rowCount++, yOffset, &Config::AllowCancellingUnleash, true);
        DrawConfigOption(rowCount++, yOffset, &Config::AllowBackgroundInput, true);
        break;
    case 2: // AUDIO
        DrawConfigOption(rowCount++, yOffset, &Config::MusicVolume, true);
        DrawConfigOption(rowCount++, yOffset, &Config::EffectsVolume, true);
        DrawConfigOption(rowCount++, yOffset, &Config::VoiceLanguage, OptionsMenu::s_pauseMenuType == SWA::eMenuType_WorldMap, cmnReason);
        DrawConfigOption(rowCount++, yOffset, &Config::Subtitles, true);
        DrawConfigOption(rowCount++, yOffset, &Config::MusicAttenuation, AudioPatches::CanAttenuate(), &Localise("Options_Desc_OSNotSupported"));
        DrawConfigOption(rowCount++, yOffset, &Config::BattleTheme, true);
        break;
    case 3: // VIDEO
        // TODO: expose WindowWidth/WindowHeight as WindowSize.
        DrawConfigOption(rowCount++, yOffset, &Config::AspectRatio, true);
        DrawConfigOption(rowCount++, yOffset, &Config::ResolutionScale, true, nullptr, 0.25f, 1.0f, 2.0f);
        DrawConfigOption(rowCount++, yOffset, &Config::Fullscreen, true);
        DrawConfigOption(rowCount++, yOffset, &Config::VSync, true);
        DrawConfigOption(rowCount++, yOffset, &Config::FPS, true, nullptr, 15, 120, 240);
        DrawConfigOption(rowCount++, yOffset, &Config::Brightness, true);
        DrawConfigOption(rowCount++, yOffset, &Config::AntiAliasing, true);
        DrawConfigOption(rowCount++, yOffset, &Config::TransparencyAntiAliasing, Config::AntiAliasing != EAntiAliasing::None, &Localise("Options_Desc_NotAvailableMSAA"));
        DrawConfigOption(rowCount++, yOffset, &Config::ShadowResolution, true);
        DrawConfigOption(rowCount++, yOffset, &Config::GITextureFiltering, true);
        DrawConfigOption(rowCount++, yOffset, &Config::MotionBlur, true);
        DrawConfigOption(rowCount++, yOffset, &Config::XboxColourCorrection, true);
        DrawConfigOption(rowCount++, yOffset, &Config::MovieScaleMode, true);
        DrawConfigOption(rowCount++, yOffset, &Config::UIScaleMode, true);
        break;
    }

    auto inputState = SWA::CInputState::GetInstance();

    bool upIsHeld = !m_lockedOnOption && (inputState->GetPadState().IsDown(SWA::eKeyState_DpadUp) ||
        inputState->GetPadState().LeftStickVertical > 0.5f);

    bool downIsHeld = !m_lockedOnOption && (inputState->GetPadState().IsDown(SWA::eKeyState_DpadDown) ||
        inputState->GetPadState().LeftStickVertical < -0.5f);

    bool scrollUp = !m_upWasHeld && upIsHeld;
    bool scrollDown = !m_downWasHeld && downIsHeld;

    int32_t prevSelectedRowIndex = m_selectedRowIndex;

    if (scrollUp)
    {
        --m_selectedRowIndex;
        if (m_selectedRowIndex < 0)
            m_selectedRowIndex = rowCount - 1;
    }
    else if (scrollDown)
    {
        ++m_selectedRowIndex;
        if (m_selectedRowIndex >= rowCount)
            m_selectedRowIndex = 0;
    }

    if (scrollUp || scrollDown)
    {
        m_rowSelectionTime = ImGui::GetTime();
        m_prevSelectedRowIndex = prevSelectedRowIndex;
        Game_PlaySound("sys_worldmap_cursor");
    }

    m_upWasHeld = upIsHeld;
    m_downWasHeld = downIsHeld;

    int32_t visibleRowCount = int32_t(floor((clipRectMax.y - clipRectMin.y) / optionHeightWithPadding));

    bool disableMoveAnimation = false;

    if (m_firstVisibleRowIndex > m_selectedRowIndex)
    {
        m_firstVisibleRowIndex = m_selectedRowIndex;
        disableMoveAnimation = true;
    }

    if (m_firstVisibleRowIndex + visibleRowCount - 1 < m_selectedRowIndex)
    {
        m_firstVisibleRowIndex = std::max(0, m_selectedRowIndex - visibleRowCount + 1);
        disableMoveAnimation = true;
    }

    if (disableMoveAnimation)
        m_prevSelectedRowIndex = m_selectedRowIndex;

    // Pop clip rect from DrawCategories
    drawList->PopClipRect();

    // Draw scroll bar
    if (rowCount > visibleRowCount)
    {
        float totalHeight = (clipRectMax.y - clipRectMin.y);
        float heightRatio = float(visibleRowCount) / float(rowCount);
        float offsetRatio = float(m_firstVisibleRowIndex) / float(rowCount);
        float minY = offsetRatio * totalHeight + clipRectMin.y;

        drawList->AddRectFilled
        (
            { clipRectMax.x, minY },
            { clipRectMax.x + gridSize, minY + totalHeight * heightRatio },
            IM_COL32(0, 128, 0, 255)
        );
    }
}

static void DrawSettingsPanel()
{
    auto drawList = ImGui::GetForegroundDrawList();

    ImVec2 settingsMin = { Scale(AlignToNextGrid(COMMON_PADDING_POS_X)), Scale(AlignToNextGrid(COMMON_PADDING_POS_Y)) };
    ImVec2 settingsMax = { Scale(AlignToNextGrid(INFO_CONTAINER_POS_X - COMMON_PADDING_POS_X)), Scale(AlignToNextGrid(720.0f - COMMON_PADDING_POS_Y)) };

    DrawContainer(settingsMin, settingsMax);

    if (DrawCategories())
    {
        DrawConfigOptions();
    }
    else
    {
        ResetSelection();
    }

    // Pop clip rect from DrawContainer
    drawList->PopClipRect();
}

static void DrawInfoPanel()
{
    auto drawList = ImGui::GetForegroundDrawList();

    ImVec2 infoMin = { Scale(AlignToNextGrid(INFO_CONTAINER_POS_X)), Scale(AlignToNextGrid(COMMON_PADDING_POS_Y)) };
    ImVec2 infoMax = { Scale(AlignToNextGrid(1280.0f - COMMON_PADDING_POS_X)), Scale(AlignToNextGrid(720.0f - COMMON_PADDING_POS_Y)) };

    DrawContainer(infoMin, infoMax);

    auto clipRectMin = drawList->GetClipRectMin();
    auto clipRectMax = drawList->GetClipRectMax();

    ImVec2 thumbnailMax = { clipRectMin.x + ScaleX(343.0f), clipRectMin.y + ScaleY(198.0f) };

    // Thumbnail box
    drawList->AddRectFilled(clipRectMin, thumbnailMax, IM_COL32(0, 0, 0, 255));

    if (m_selectedItem)
    {
        auto desc = m_selectedItem->GetDescription();

        if (m_inaccessibleReason)
        {
            desc = *m_inaccessibleReason;
        }
        else
        {
            // Specialised description for resolution scale
            if (m_selectedItem->GetName() == "ResolutionScale")
            {
                char buf[100];
                auto resScale = round(*(float*)m_selectedItem->GetValue() * 1000) / 1000;

                std::snprintf(buf, sizeof(buf), desc.c_str(),
                    (int)((float)Window::s_width * resScale),
                    (int)((float)Window::s_height * resScale));

                desc = buf;
            }

            desc += "\n\n" + m_selectedItem->GetValueDescription();
        }

        auto size = Scale(26.0f);

        drawList->AddText
        (
            m_seuratFont,
            size,
            { clipRectMin.x, thumbnailMax.y + size - 5.0f },
            IM_COL32_WHITE,
            desc.c_str(),
            0,
            clipRectMax.x - clipRectMin.x
        );
    }

    // Pop clip rect from DrawContainer
    drawList->PopClipRect();
}

void OptionsMenu::Init()
{
    auto& io = ImGui::GetIO();

    constexpr float FONT_SCALE = 2.0f;

    m_seuratFont = io.Fonts->AddFontFromFileTTF("FOT-SeuratPro-M.otf", 26.0f * FONT_SCALE);
    m_dfsogeistdFont = io.Fonts->AddFontFromFileTTF("DFSoGeiStd-W7.otf", 48.0f * FONT_SCALE);
    m_newRodinFont = io.Fonts->AddFontFromFileTTF("FOT-NewRodinPro-DB.otf", 20.0f * FONT_SCALE);
}

void OptionsMenu::Draw()
{
    auto pInputState = SWA::CInputState::GetInstance();

    if (!s_isVisible)
        return;

    // We've entered the menu now, no need to check this.
    if (pInputState->GetPadState().IsReleased(SWA::eKeyState_A))
        m_isEnterKeyBuffered = false;

    g_callbackDataIndex = 0;
    
    auto& res = ImGui::GetIO().DisplaySize;
    auto drawList = ImGui::GetForegroundDrawList();
    
    if (s_isPause && s_pauseMenuType != SWA::eMenuType_WorldMap)
        drawList->AddRectFilled({ 0.0f, 0.0f }, res, IM_COL32(0, 0, 0, 223));

    DrawScanlineBars();
    DrawSettingsPanel();
    DrawInfoPanel();
}

void OptionsMenu::Open(bool isPause, SWA::EMenuType pauseMenuType)
{
    s_isVisible = true;
    s_isPause = isPause;

    s_pauseMenuType = pauseMenuType;

    m_appearTime = ImGui::GetTime();
    m_categoryIndex = 0;
    m_categoryAnimMin = { 0.0f, 0.0f };
    m_categoryAnimMax = { 0.0f, 0.0f };
    m_selectedItem = nullptr;

    /* Store button state so we can track it later
       and prevent the first item being selected. */
    if (SWA::CInputState::GetInstance()->GetPadState().IsDown(SWA::eKeyState_A))
        m_isEnterKeyBuffered = true;

    ResetSelection();

    *(bool*)g_memory.Translate(0x8328BB26) = false;

    // TODO: animate Miles Electric in if we're in a stage.
}

void OptionsMenu::Close()
{
    s_isVisible = false;

    *(bool*)g_memory.Translate(0x8328BB26) = true;

    Config::Save();

    // TODO: animate Miles Electric out if we're in a stage.
}

bool OptionsMenu::CanClose()
{
    return !m_lockedOnOption;
}
