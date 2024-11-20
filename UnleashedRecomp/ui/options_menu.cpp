#include "options_menu.h"
#include "window.h"
#include "exports.h"

#include <api/SWA/System/InputState.h>
#include <gpu/imgui_common.h>
#include <kernel/heap.h>
#include <kernel/memory.h>

constexpr float COMMON_PADDING_POS_Y = 118.0f;
constexpr float COMMON_PADDING_POS_X = 30.0f;
constexpr float INFO_CONTAINER_POS_X = 870.0f;

static ImFont* g_seuratFont;
static ImFont* g_dfsogeistdFont;
static ImFont* g_newRodinFont;

static const IConfigDef* g_selectedItem;

static bool g_isEnterKeyBuffered = false;
static double g_appearTime = 0.0;

void OptionsMenu::Init()
{
    auto& io = ImGui::GetIO();

    constexpr float FONT_SCALE = 2.0f;

    g_seuratFont = io.Fonts->AddFontFromFileTTF("FOT-SeuratPro-M.otf", 26.0f * FONT_SCALE);
    g_dfsogeistdFont = io.Fonts->AddFontFromFileTTF("DFSoGeiStd-W7.otf", 48.0f * FONT_SCALE);
    g_newRodinFont = io.Fonts->AddFontFromFileTTF("FOT-NewRodinPro-DB.otf", 20.0f * FONT_SCALE);
}

static std::vector<std::unique_ptr<ImGuiCallbackData>> g_callbackData;
static uint32_t g_callbackDataIndex = 0;

static ImGuiCallbackData* AddCallback(ImGuiCallback callback)
{
    if (g_callbackDataIndex >= g_callbackData.size())
        g_callbackData.emplace_back(std::make_unique<ImGuiCallbackData>());

    auto& callbackData = g_callbackData[g_callbackDataIndex];
    ++g_callbackDataIndex;

    ImGui::GetForegroundDrawList()->AddCallback(reinterpret_cast<ImDrawCallback>(callback), callbackData.get());

    return callbackData.get();
}

static void SetGradient(const ImVec2& min, const ImVec2& max, ImU32 top, ImU32 bottom)
{
    auto callbackData = AddCallback(ImGuiCallback::SetGradient);
    callbackData->setGradient.gradientMin[0] = min.x;
    callbackData->setGradient.gradientMin[1] = min.y; 
    callbackData->setGradient.gradientMax[0] = max.x;
    callbackData->setGradient.gradientMax[1] = max.y;  
    callbackData->setGradient.gradientTop = top;
    callbackData->setGradient.gradientBottom = bottom;
}

static void ResetGradient()
{
    auto callbackData = AddCallback(ImGuiCallback::SetGradient);
    memset(&callbackData->setGradient, 0, sizeof(callbackData->setGradient));
}

static void SetShaderModifier(uint32_t shaderModifier)
{
    auto callbackData = AddCallback(ImGuiCallback::SetShaderModifier);
    callbackData->setShaderModifier.shaderModifier = shaderModifier;
}

static void DrawTextWithMarquee(const ImFont* font, float fontSize, const ImVec2& pos, const ImVec2& min, const ImVec2& max, ImU32 color, const char* text, double time, double delay, double speed)
{
    auto drawList = ImGui::GetForegroundDrawList();
    auto rectWidth = max.x - min.x;
    auto textSize = g_seuratFont->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, text);
    auto textX = pos.x - fmodf(std::max(0.0, ImGui::GetTime() - (time + delay)) * speed, textSize.x + rectWidth);

    drawList->PushClipRect(min, max, true);

    if (textX <= pos.x)
        drawList->AddText(font, fontSize, { textX, pos.y }, color, text);

    if (textX + textSize.x < pos.x)
        drawList->AddText(font, fontSize, { textX + textSize.x + rectWidth, pos.y }, color, text);

    drawList->PopClipRect();
}

static void DrawTextWithOutline(const ImFont* font, float fontSize, const ImVec2& pos, ImU32 color, const char* text, int32_t outlineSize, ImU32 outlineColor)
{
    auto drawList = ImGui::GetForegroundDrawList();

    // TODO: This is very inefficient!
    for (int32_t i = -outlineSize + 1; i < outlineSize; i++)
    {
        for (int32_t j = -outlineSize + 1; j < outlineSize; j++)
        {
            drawList->AddText(font, fontSize, { pos.x + i, pos.y + j }, outlineColor, text);
        }
    }

    drawList->AddText(font, fontSize, pos, color, text);
}

// Not aspect ratio aware. Will stretch.
static float ScaleX(float x)
{
    auto& io = ImGui::GetIO();
    return x * io.DisplaySize.x / 1280.0f;
}

static float ScaleY(float y)
{
    auto& io = ImGui::GetIO();
    return y * io.DisplaySize.y / 720.0f;
}

// Aspect ratio aware.
static float Scale(float size)
{
    auto& io = ImGui::GetIO();
    if (io.DisplaySize.x > io.DisplaySize.y)
        return size * std::max(1.0f, io.DisplaySize.y / 720.0f);
    else
        return size * std::max(1.0f, io.DisplaySize.x / 1280.0f);
}

static float Lerp(float a, float b, float t)
{
    return a + (b - a) * t;
}

static ImVec2 Lerp(const ImVec2& a, const ImVec2& b, float t)
{
    return { a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t };
}

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

    if (OptionsMenu::s_isStage)
    {
        // Top bar fade
        drawList->AddRectFilledMultiColor(
            { 0.0f, 0.0f },
            { res.x, height },
            FADE_COLOR0,
            FADE_COLOR0,
            FADE_COLOR1,
            FADE_COLOR1);

        // Bottom bar fade
        drawList->AddRectFilledMultiColor(
            { res.x, res.y },
            { 0.0f, res.y - height },
            FADE_COLOR0,
            FADE_COLOR0,
            FADE_COLOR1,
            FADE_COLOR1);
    }

    SetShaderModifier(IMGUI_SHADER_MODIFIER_SCANLINE);

    // Top bar
    drawList->AddRectFilledMultiColor(
        { 0.0f, 0.0f },
        { res.x, height },
        COLOR0,
        COLOR0,
        COLOR1,
        COLOR1);

    // Bottom bar
    drawList->AddRectFilledMultiColor(
        { res.x, res.y },
        { 0.0f, res.y - height },
        COLOR0,
        COLOR0,
        COLOR1,
        COLOR1);

    SetShaderModifier(IMGUI_SHADER_MODIFIER_NONE);

    // Options text
    DrawTextWithOutline(g_dfsogeistdFont, Scale(48.0f), { Scale(122.0f), Scale(56.0f) }, IM_COL32(255, 195, 0, 255), "OPTIONS", Scale(4), IM_COL32_BLACK);

    // Top bar line
    drawList->AddLine(
        { 0.0f, height },
        { res.x, height },
        OUTLINE_COLOR,
        Scale(1));

    // Bottom bar line
    drawList->AddLine(
        { 0.0f, res.y - height },
        { res.x, res.y - height },
        OUTLINE_COLOR,
        Scale(1));
}

static constexpr size_t GRID_SIZE = 9;

static float AlignToNextGrid(float value)
{
    return floor(value / GRID_SIZE) * GRID_SIZE;
}

static double ComputeMotion(double frameOffset, double frames)
{
    double t = std::clamp((ImGui::GetTime() - g_appearTime - frameOffset / 60.0) / frames * 60.0, 0.0, 1.0);
    return sqrt(t);
}

// all in 60 FPS
static constexpr double CONTAINER_LINE_ANIMATION_DURATION = 8.0;

static constexpr double CONTAINER_OUTER_TIME = CONTAINER_LINE_ANIMATION_DURATION + 8.0; // 8 frame delay
static constexpr double CONTAINER_OUTER_DURATION = 8.0;

static constexpr double CONTAINER_INNER_TIME = CONTAINER_OUTER_TIME + CONTAINER_OUTER_DURATION + 8.0; // 8 frame delay
static constexpr double CONTAINER_INNER_DURATION = 8.0;

static constexpr double CONTAINER_BACKGROUND_TIME = CONTAINER_INNER_TIME + CONTAINER_INNER_DURATION + 8.0; // 8 frame delay
static constexpr double CONTAINER_BACKGROUND_DURATION = 12.0;

static constexpr double CONTAINER_FULL_DURATION = CONTAINER_BACKGROUND_TIME + CONTAINER_BACKGROUND_DURATION;

static void DrawContainer(ImVec2 min, ImVec2 max)
{
    double containerHeight = ComputeMotion(0.0, CONTAINER_LINE_ANIMATION_DURATION);

    float center = (min.y + max.y) / 2.0f;
    min.y = Lerp(center, min.y, containerHeight);
    max.y = Lerp(center, max.y, containerHeight);

    auto& res = ImGui::GetIO().DisplaySize;
    auto drawList = ImGui::GetForegroundDrawList();


    double outerAlpha = ComputeMotion(CONTAINER_OUTER_TIME, CONTAINER_OUTER_DURATION);
    double innerAlpha = ComputeMotion(CONTAINER_INNER_TIME, CONTAINER_INNER_DURATION);
    double backgroundAlpha = ComputeMotion(CONTAINER_BACKGROUND_TIME, CONTAINER_BACKGROUND_DURATION);

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

// TODO: localise this.
static constexpr const char* CATEGORIES[] =
{
    "SYSTEM",
    "INPUT",
    "AUDIO",
    "VIDEO"
};

static int32_t g_categoryIndex;
static ImVec2 g_categoryAnimMin;
static ImVec2 g_categoryAnimMax;

static int32_t g_firstVisibleRowIndex;
static int32_t g_prevSelectedRowIndex;
static int32_t g_selectedRowIndex;
static double g_rowSelectionTime;

static bool g_leftWasHeld;
static bool g_upWasHeld;
static bool g_rightWasHeld;
static bool g_downWasHeld;

static bool g_lockedOnOption;
static double g_lastTappedTime;
static double g_lastIncrementTime;
static double g_lastIncrementSoundTime;

static void ResetSelection()
{
    g_firstVisibleRowIndex = 0;
    g_selectedRowIndex = 0;
    g_prevSelectedRowIndex = 0;
    g_rowSelectionTime = ImGui::GetTime();
    g_leftWasHeld = false;
    g_upWasHeld = false;
    g_rightWasHeld = false;
    g_downWasHeld = false;
}

static constexpr double CONTAINER_CATEGORY_TIME = (CONTAINER_INNER_TIME + CONTAINER_BACKGROUND_TIME) / 2.0;
static constexpr double CONTAINER_CATEGORY_DURATION = 12.0;

static bool DrawCategories()
{
    double motion = ComputeMotion(CONTAINER_CATEGORY_TIME, CONTAINER_CATEGORY_DURATION);
    if (motion == 0.0)
        return false;

    auto inputState = SWA::CInputState::GetInstance();

    bool moveLeft = !g_lockedOnOption && (inputState->GetPadState().IsTapped(SWA::eKeyState_LeftBumper) ||
        inputState->GetPadState().IsTapped(SWA::eKeyState_LeftTrigger));

    bool moveRight = !g_lockedOnOption && (inputState->GetPadState().IsTapped(SWA::eKeyState_RightBumper) ||
        inputState->GetPadState().IsTapped(SWA::eKeyState_RightTrigger));

    if (moveLeft)
    {
        --g_categoryIndex;
        if (g_categoryIndex < 0)
            g_categoryIndex = std::size(CATEGORIES) - 1;
    }
    else if (moveRight)
    {
        ++g_categoryIndex;
        if (g_categoryIndex >= std::size(CATEGORIES))
            g_categoryIndex = 0;
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
    ImVec2 textSizes[std::size(CATEGORIES)];
    float tabWidthSum = 0.0f;
    for (size_t i = 0; i < std::size(CATEGORIES); i++)
    {
        textSizes[i] = g_dfsogeistdFont->CalcTextSizeA(size, FLT_MAX, 0.0f, CATEGORIES[i]);
        tabWidthSum += textSizes[i].x + textPadding * 2.0f;
    }
    tabWidthSum += (std::size(CATEGORIES) - 1) * tabPadding;

    float tabHeight = gridSize * 4.0f;
    float xOffset = ((clipRectMax.x - clipRectMin.x) - tabWidthSum) / 2.0f;
    xOffset -= (1.0 - motion) * gridSize * 4.0;

    ImVec2 minVec[std::size(CATEGORIES)];

    for (size_t i = 0; i < std::size(CATEGORIES); i++)
    {
        ImVec2 min = { clipRectMin.x + xOffset, clipRectMin.y };

        xOffset += textSizes[i].x + textPadding * 2.0f;
        ImVec2 max = { clipRectMin.x + xOffset, clipRectMin.y + tabHeight };
        xOffset += tabPadding;

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

                g_categoryAnimMin = Lerp(g_categoryAnimMin, min, 1.0f - exp(-16.0f * ImGui::GetIO().DeltaTime));
                g_categoryAnimMax = { g_categoryAnimMin.x + animWidth, g_categoryAnimMin.y + height };
            }

            SetShaderModifier(IMGUI_SHADER_MODIFIER_SCANLINE_BUTTON);

            drawList->AddRectFilledMultiColor(
                g_categoryAnimMin,
                g_categoryAnimMax,
                IM_COL32(0, 130, 0, 223 * motion),
                IM_COL32(0, 130, 0, 178 * motion), 
                IM_COL32(0, 130, 0, 223 * motion),
                IM_COL32(0, 130, 0, 178 * motion));

            drawList->AddRectFilledMultiColor(
                g_categoryAnimMin, 
                g_categoryAnimMax,
                IM_COL32(0, 0, 0, 13 * motion),
                IM_COL32(0, 0, 0, 0),
                IM_COL32(0, 0, 0, 55 * motion),
                IM_COL32(0, 0, 0, 6));

            drawList->AddRectFilledMultiColor(
                g_categoryAnimMin, 
                g_categoryAnimMax,
                IM_COL32(0, 130, 0, 13 * motion),
                IM_COL32(0, 130, 0, 111 * motion),
                IM_COL32(0, 130, 0, 0), 
                IM_COL32(0, 130, 0, 55 * motion));

            SetShaderModifier(IMGUI_SHADER_MODIFIER_NONE);
        }

        min.x += textPadding;

        // Store to draw again later, otherwise the tab background gets drawn on top of text during the animation.
        minVec[i] = min; 
    }

    for (size_t i = 0; i < std::size(CATEGORIES); i++)
    {
        auto& min = minVec[i];
        uint8_t alpha = (i == g_categoryIndex ? 235 : 128) * motion;

        SetGradient(
            min,
            { min.x + textSizes[i].x, min.y + textSizes[i].y },
            IM_COL32(128, 255, 0, alpha),
            IM_COL32(255, 192, 0, alpha));

        DrawTextWithOutline(
            g_dfsogeistdFont,
            size,
            min,
            IM_COL32_WHITE,
            CATEGORIES[i],
            Scale(3),
            IM_COL32_BLACK);

        ResetGradient();
    }

    if ((ImGui::GetTime() - g_appearTime) >= (CONTAINER_FULL_DURATION / 60.0))
    {
        drawList->PushClipRect({ clipRectMin.x, clipRectMin.y + gridSize * 6.0f }, { clipRectMax.x - gridSize, clipRectMax.y - gridSize });
        return true;
    }

    return false;
}

// extern definition to avoid including "video.h", the header is quite large
extern void VideoConfigValueChangedCallback(IConfigDef* config);

template<typename T>
static void DrawConfigOption(int32_t rowIndex, float yOffset, ConfigDef<T>* config, T valueMin = T(0), T valueCenter = T(0.5), T valueMax = T(1))
{
    auto drawList = ImGui::GetForegroundDrawList();
    auto clipRectMin = drawList->GetClipRectMin();
    auto clipRectMax = drawList->GetClipRectMax();
    auto& padState = SWA::CInputState::GetInstance()->GetPadState();

    constexpr ImU32 COLOR0 = IM_COL32(0xE2, 0x71, 0x22, 0x80);
    constexpr ImU32 COLOR1 = IM_COL32(0x92, 0xFF, 0x31, 0x80);

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
    auto textSize = g_seuratFont->CalcTextSizeA(size, FLT_MAX, 0.0f, configName.c_str());

    ImVec2 textPos = { min.x + gridSize, min.y + (optionHeight - textSize.y) / 2.0f };
    ImVec4 textClipRect = { min.x, min.y, max.x, max.y };

    bool lockedOnOption = false;
    if (g_selectedRowIndex == rowIndex)
    {
        g_selectedItem = config;

        if (!g_isEnterKeyBuffered)
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
                    g_lockedOnOption ^= true;
                    if (g_lockedOnOption)
                    {
                        g_leftWasHeld = false;
                        g_rightWasHeld = false;
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
                    g_lockedOnOption = false;

                    Game_PlaySound("sys_worldmap_cansel");
                }

                lockedOnOption = g_lockedOnOption;
            }
        }
    }

    bool fadedOut = g_lockedOnOption && g_selectedItem != config;
    float alpha = fadedOut ? 0.5f : 1.0f;

    if (g_selectedItem == config)
    {
        float prevItemOffset = (g_prevSelectedRowIndex - g_selectedRowIndex) * (optionHeight + optionPadding);
        double animRatio = std::clamp((ImGui::GetTime() - g_rowSelectionTime) * 60.0 / 8.0, 0.0, 1.0);
        prevItemOffset *= pow(1.0 - animRatio, 3.0);

        drawList->AddRectFilledMultiColor({ min.x, min.y + prevItemOffset }, { max.x, max.y + prevItemOffset }, COLOR0, COLOR0, COLOR1, COLOR1);

        DrawTextWithMarquee(g_seuratFont, size, textPos, min, max, IM_COL32_WHITE, configName.c_str(), g_rowSelectionTime, 0.9, 250.0);
    }
    else
    {
        drawList->AddText(g_seuratFont, size, textPos, IM_COL32(255, 255, 255, 255 * alpha), configName.c_str(), 0, 0.0f, &textClipRect);
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

        drawList->AddRectFilledMultiColor(
            { min.x + xPadding, min.y + yPadding }, 
            { max.x - xPadding, max.y - yPadding }, 
            innerColor0, 
            innerColor0, 
            innerColor1, 
            innerColor1);

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
        drawList->AddTriangleFilled(
            { min.x - trianglePadding, min.y },
            { min.x - trianglePadding, max.y }, 
            { min.x - trianglePadding - triangleWidth, (min.y + max.y) / 2.0f }, 
            COLOR);

        // Right
        drawList->AddTriangleFilled(
            { max.x + trianglePadding, max.y },
            { max.x + trianglePadding, min.y },
            { max.x + trianglePadding + triangleWidth, (min.y + max.y) / 2.0f },
            COLOR);

        bool leftIsHeld = padState.IsDown(SWA::eKeyState_DpadLeft) || padState.LeftStickHorizontal < -0.5f;
        bool rightIsHeld = padState.IsDown(SWA::eKeyState_DpadRight) || padState.LeftStickHorizontal > 0.5f;

        bool leftTapped = !g_leftWasHeld && leftIsHeld;
        bool rightTapped = !g_rightWasHeld && rightIsHeld;

        double time = ImGui::GetTime();

        if (leftTapped || rightTapped)
            g_lastTappedTime = time;

        bool decrement = leftTapped;
        bool increment = rightTapped;

        bool fastIncrement = (time - g_lastTappedTime) > 0.5;
        constexpr double INCREMENT_TIME = 1.0 / 120.0;

        constexpr double INCREMENT_SOUND_TIME = 1.0 / 7.5;
        bool isPlayIncrementSound = true;

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

        g_leftWasHeld = leftIsHeld;
        g_rightWasHeld = rightIsHeld;

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
            } while (fastIncrement && deltaTime > 0.0f);

            bool isConfigValueInBounds = config->Value >= valueMin && config->Value <= valueMax;

            if ((increment || decrement) && isConfigValueInBounds && isPlayIncrementSound) 
            {
                g_lastIncrementSoundTime = time;
                Game_PlaySound("sys_actstg_twn_speechbutton");
            }

            config->Value = std::clamp(config->Value, valueMin, valueMax);
        }

        if (config->Callback)
            config->Callback(config);
    }

    std::string valueText;
    if constexpr (std::is_same_v<T, float>)
        valueText = std::format("{}%", int32_t(round(config->Value * 100.0f)));
    else if constexpr (std::is_same_v<T, int32_t>)
        valueText = config->Value >= valueMax ? "MAX" : std::format("{}", config->Value);
    else
        valueText = config->GetValueLocalised();

    size = Scale(20.0f);
    textSize = g_newRodinFont->CalcTextSizeA(size, FLT_MAX, 0.0f, valueText.data());

    min.x += ((max.x - min.x) - textSize.x) / 2.0f;
    min.y += ((max.y - min.y) - textSize.y) / 2.0f;

    SetGradient(
        min,
        { min.x + textSize.x, min.y + textSize.y },
        IM_COL32(192, 255, 0, 255),
        IM_COL32(128, 170, 0, 255)
    );

    DrawTextWithOutline(
        g_newRodinFont,
        size,
        min,
        IM_COL32(255, 255, 255, 255 * alpha),
        valueText.data(),
        Scale(2),
        IM_COL32(0, 0, 0, 255 * alpha));

    ResetGradient();
}

static void DrawConfigOptions()
{
    auto drawList = ImGui::GetForegroundDrawList();
    auto clipRectMin = drawList->GetClipRectMin();
    auto clipRectMax = drawList->GetClipRectMax();

    g_selectedItem = nullptr;

    float gridSize = Scale(GRID_SIZE);
    float optionHeightWithPadding = gridSize * 6.0f;
    float yOffset = -g_firstVisibleRowIndex * optionHeightWithPadding;

    int32_t rowCount = 0;

    // TODO: Don't use raw numbers here!
    switch (g_categoryIndex)
    {
    case 0: // SYSTEM
        DrawConfigOption(rowCount++, yOffset, &Config::Language);
        DrawConfigOption(rowCount++, yOffset, &Config::Hints);
        DrawConfigOption(rowCount++, yOffset, &Config::ControlTutorial);
        DrawConfigOption(rowCount++, yOffset, &Config::SaveScoreAtCheckpoints);
        DrawConfigOption(rowCount++, yOffset, &Config::UnleashGaugeBehaviour);
        DrawConfigOption(rowCount++, yOffset, &Config::WerehogHubTransformVideo);
        DrawConfigOption(rowCount++, yOffset, &Config::LogoSkip);
        break;
    case 1: // INPUT
        DrawConfigOption(rowCount++, yOffset, &Config::CameraXInvert);
        DrawConfigOption(rowCount++, yOffset, &Config::CameraYInvert);
        DrawConfigOption(rowCount++, yOffset, &Config::XButtonHoming);
        DrawConfigOption(rowCount++, yOffset, &Config::UnleashCancel);
        DrawConfigOption(rowCount++, yOffset, &Config::BackgroundInput);
        break;
    case 2: // AUDIO
        DrawConfigOption(rowCount++, yOffset, &Config::MusicVolume);
        DrawConfigOption(rowCount++, yOffset, &Config::SEVolume);
        DrawConfigOption(rowCount++, yOffset, &Config::VoiceLanguage);
        DrawConfigOption(rowCount++, yOffset, &Config::Subtitles);
        DrawConfigOption(rowCount++, yOffset, &Config::WerehogBattleMusic);
        break;
    case 3: // VIDEO
        // TODO: expose WindowWidth/WindowHeight as WindowSize.
        DrawConfigOption(rowCount++, yOffset, &Config::ResolutionScale, 0.25f, 1.0f, 2.0f);
        DrawConfigOption(rowCount++, yOffset, &Config::Fullscreen);
        DrawConfigOption(rowCount++, yOffset, &Config::VSync);
        DrawConfigOption(rowCount++, yOffset, &Config::TripleBuffering);
        DrawConfigOption(rowCount++, yOffset, &Config::FPS, 15, 120, 240);
        DrawConfigOption(rowCount++, yOffset, &Config::Brightness);
        DrawConfigOption(rowCount++, yOffset, &Config::AntiAliasing);
        DrawConfigOption(rowCount++, yOffset, &Config::AlphaToCoverage);
        DrawConfigOption(rowCount++, yOffset, &Config::ShadowResolution);
        DrawConfigOption(rowCount++, yOffset, &Config::GITextureFiltering);
        DrawConfigOption(rowCount++, yOffset, &Config::MotionBlur);
        DrawConfigOption(rowCount++, yOffset, &Config::Xbox360ColourCorrection);
        DrawConfigOption(rowCount++, yOffset, &Config::MovieScaleMode);
        DrawConfigOption(rowCount++, yOffset, &Config::UIScaleMode);
        break;
    }

    auto inputState = SWA::CInputState::GetInstance();

    bool upIsHeld = !g_lockedOnOption && (inputState->GetPadState().IsDown(SWA::eKeyState_DpadUp) ||
        inputState->GetPadState().LeftStickVertical > 0.5f);

    bool downIsHeld = !g_lockedOnOption && (inputState->GetPadState().IsDown(SWA::eKeyState_DpadDown) ||
        inputState->GetPadState().LeftStickVertical < -0.5f);

    bool scrollUp = !g_upWasHeld && upIsHeld;
    bool scrollDown = !g_downWasHeld && downIsHeld;

    int32_t prevSelectedRowIndex = g_selectedRowIndex;

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
        g_rowSelectionTime = ImGui::GetTime();
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
    }

    if (g_firstVisibleRowIndex + visibleRowCount - 1 < g_selectedRowIndex)
    {
        g_firstVisibleRowIndex = std::max(0, g_selectedRowIndex - visibleRowCount + 1);
        disableMoveAnimation = true;
    }

    if (disableMoveAnimation)
        g_prevSelectedRowIndex = g_selectedRowIndex;

    // Pop clip rect from DrawCategories
    drawList->PopClipRect();

    // Draw scroll bar
    if (rowCount > visibleRowCount)
    {
        float totalHeight = (clipRectMax.y - clipRectMin.y);
        float heightRatio = float(visibleRowCount) / float(rowCount);
        float offsetRatio = float(g_firstVisibleRowIndex) / float(rowCount);
        float minY = offsetRatio * totalHeight + clipRectMin.y;

        drawList->AddRectFilled(
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
        DrawConfigOptions();
    else
        ResetSelection();

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

    ImVec2 thumbnailMax = { clipRectMin.x + ScaleX(343.0f), clipRectMin.y + ScaleY(193.0f) };

    // Thumbnail box
    drawList->AddRectFilled(clipRectMin, thumbnailMax, IM_COL32(0, 0, 0, 255));

    if (g_selectedItem)
    {
        auto desc = g_selectedItem->GetDescription();

        // Specialised description for resolution scale
        if (g_selectedItem->GetName() == "ResolutionScale")
        {
            char buf[100];
            auto resScale = round(*(float*)g_selectedItem->GetValue() * 1000) / 1000;

            std::snprintf(buf, sizeof(buf), desc.c_str(),
                (int)((float)Window::s_width * resScale),
                (int)((float)Window::s_height * resScale));

            desc = buf;
        }

        desc += "\n\n" + g_selectedItem->GetValueDescription();

        auto size = Scale(26.0f);
        auto textSize = g_seuratFont->CalcTextSizeA(size, FLT_MAX, 0.0f, desc.c_str());

        // TODO: fix word wrap width not scaling to resolution.
        drawList->AddText(
            g_seuratFont,
            size,
            { clipRectMin.x, thumbnailMax.y + size },
            IM_COL32_WHITE,
            desc.c_str(),
            0,
            thumbnailMax.x - clipRectMin.x
        );

        // Pop clip rect from DrawContainer
        drawList->PopClipRect();
    }
}

void OptionsMenu::Draw()
{
    auto pInputState = SWA::CInputState::GetInstance();

    if (!s_isVisible || pInputState->GetPadState().IsDown(SWA::eKeyState_Y))
        return;

    // We've entered the menu now, no need to check this.
    if (pInputState->GetPadState().IsReleased(SWA::eKeyState_A))
        g_isEnterKeyBuffered = false;

    g_callbackDataIndex = 0;
    
    auto& res = ImGui::GetIO().DisplaySize;
    auto drawList = ImGui::GetForegroundDrawList();
    
    if (s_isStage)
        drawList->AddRectFilled({ 0.0f, 0.0f }, res, IM_COL32(0, 0, 0, 223));

    DrawScanlineBars();
    DrawSettingsPanel();
    DrawInfoPanel();
}

void OptionsMenu::Open(bool stage)
{
    s_isVisible = true;
    s_isStage = stage;
    g_appearTime = ImGui::GetTime();
    g_categoryIndex = 0;
    g_categoryAnimMin = { 0.0f, 0.0f };
    g_categoryAnimMax = { 0.0f, 0.0f };

    /* Store button state so we can track it later
       and prevent the first item being selected. */
    if (SWA::CInputState::GetInstance()->GetPadState().IsDown(SWA::eKeyState_A))
        g_isEnterKeyBuffered = true;

    ResetSelection();

    *(bool*)g_memory.Translate(0x8328BB26) = false;

    // TODO: animate Miles Electric in if we're in a stage.
}

void OptionsMenu::Close(bool stage)
{
    s_isVisible = false;
    s_isStage = stage;

    *(bool*)g_memory.Translate(0x8328BB26) = true;

    // TODO: animate Miles Electric out if we're in a stage.
}

bool OptionsMenu::CanClose()
{
    return !g_lockedOnOption;
}
