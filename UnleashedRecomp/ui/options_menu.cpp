#include "options_menu.h"
#include "window.h"
#include <kernel/memory.h>
#include <gpu/imgui_common.h>
#include <api/SWA/System/InputState.h>

constexpr float COMMON_PADDING_POS_Y = 118.0f;
constexpr float COMMON_PADDING_POS_X = 30.0f;
constexpr float INFO_CONTAINER_POS_X = 870.0f;

static ImFont* g_seuratFont;
static ImFont* g_dfsogeistdFont;
static ImFont* g_newRodinFont;

static const IConfigDef* g_selectedItem;

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

// TODO: add delay argument to delay when to start the marquee. Unleashed menus have ~0.9 seconds of delay before the text beings to marquee.
// TODO: reset the marquee position when a different item is selected. This might have to be handled once we use events for controller input.
static void DrawTextWithMarquee(const ImFont* font, float fontSize, const ImVec2& pos, const ImVec2& min, const ImVec2& max, ImU32 color, const char* text, float speed)
{
    auto drawList = ImGui::GetForegroundDrawList();
    auto rectWidth = max.x - min.x;
    auto textSize = g_seuratFont->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, text);
    auto textX = pos.x - fmodf(speed * ImGui::GetTime(), textSize.x + rectWidth);

    drawList->PushClipRect(min, max, true);

    if (textX < pos.x)
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

static void DrawContainer(const ImVec2& min, const ImVec2& max)
{
    auto& res = ImGui::GetIO().DisplaySize;
    auto drawList = ImGui::GetForegroundDrawList();

    constexpr uint32_t BACKGROUND_COLOR = IM_COL32(0, 0, 0, 223);
    constexpr uint32_t COLOR = IM_COL32(0, 49, 0, 255);
    constexpr uint32_t INNER_COLOR = IM_COL32(0, 33, 0, 255);
    constexpr uint32_t LINE_COLOR = IM_COL32(0, 89, 0, 255);

    float gridSize = Scale(GRID_SIZE);

    drawList->AddRectFilled(min, max, BACKGROUND_COLOR); // Background

    SetShaderModifier(IMGUI_SHADER_MODIFIER_CHECKERBOARD);

    drawList->AddRectFilled(min, max, COLOR); // Container
    drawList->AddRectFilled({ min.x + gridSize, min.y + gridSize }, { max.x - gridSize, max.y - gridSize }, INNER_COLOR); // Inner container

    SetShaderModifier(IMGUI_SHADER_MODIFIER_NONE);

    float lineSize = Scale(2);

    // Top line
    drawList->AddLine({ min.x + gridSize, min.y + gridSize }, { min.x + gridSize, min.y + gridSize * 2.0f }, LINE_COLOR, lineSize); // Vertical left
    drawList->AddLine({ min.x + gridSize, min.y + gridSize }, { max.x - gridSize, min.y + gridSize }, LINE_COLOR, lineSize); // Horizontal
    drawList->AddLine({ max.x - gridSize, min.y + gridSize }, { max.x - gridSize, min.y + gridSize * 2.0f }, LINE_COLOR, lineSize); // Vertical right

    // Bottom line
    drawList->AddLine({ min.x + gridSize, max.y - gridSize }, { min.x + gridSize, max.y - gridSize * 2.0f }, LINE_COLOR, lineSize); // Vertical left
    drawList->AddLine({ min.x + gridSize, max.y - gridSize }, { max.x - gridSize, max.y - gridSize }, LINE_COLOR, lineSize); // Horizontal
    drawList->AddLine({ max.x - gridSize, max.y - gridSize }, { max.x - gridSize, max.y - gridSize * 2.0f }, LINE_COLOR, lineSize); // Vertical right

    // The draw area
    drawList->PushClipRect({ min.x + gridSize * 2.0f, min.y + gridSize * 2.0f }, { max.x - gridSize * 2.0f + 1.0f, max.y - gridSize * 2.0f + 1.0f });
}

static constexpr const char* CATEGORIES[] =
{
    "SYSTEM",
    "CONTROLS",
    "AUDIO",
    "VIDEO"
};

static int32_t g_categoryIndex;
static int32_t g_firstVisibleRowIndex;
static int32_t g_selectedRowIndex;

static void DrawCategories()
{
    auto inputState = SWA::CInputState::GetInstance();

    bool moveLeft = inputState->GetPadState().IsTapped(SWA::eKeyState_LeftBumper) ||
        inputState->GetPadState().IsTapped(SWA::eKeyState_LeftTrigger);

    bool moveRight = inputState->GetPadState().IsTapped(SWA::eKeyState_RightBumper) ||
        inputState->GetPadState().IsTapped(SWA::eKeyState_RightTrigger);

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
        g_firstVisibleRowIndex = 0;
        g_selectedRowIndex = 0;
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

    for (size_t i = 0; i < std::size(CATEGORIES); i++)
    {
        ImVec2 min = { clipRectMin.x + xOffset, clipRectMin.y };

        xOffset += textSizes[i].x + textPadding * 2.0f;
        ImVec2 max = { clipRectMin.x + xOffset, clipRectMin.y + tabHeight };
        xOffset += tabPadding;

        uint32_t alpha = 235;

        if (g_categoryIndex == i)
        {
            SetShaderModifier(IMGUI_SHADER_MODIFIER_SCANLINE_BUTTON);

            drawList->AddRectFilledMultiColor(min, max, IM_COL32(0, 130, 0, 223), IM_COL32(0, 130, 0, 178), IM_COL32(0, 130, 0, 223), IM_COL32(0, 130, 0, 178));
            drawList->AddRectFilledMultiColor(min, max, IM_COL32(0, 0, 0, 13), IM_COL32(0, 0, 0, 0), IM_COL32(0, 0, 0, 55), IM_COL32(0, 0, 0, 6));
            drawList->AddRectFilledMultiColor(min, max, IM_COL32(0, 130, 0, 13), IM_COL32(0, 130, 0, 111), IM_COL32(0, 130, 0, 0), IM_COL32(0, 130, 0, 55));

            SetShaderModifier(IMGUI_SHADER_MODIFIER_NONE);
        }
        else
        {
            alpha = 128;
        }

        min.x += textPadding;

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

    drawList->PushClipRect({ clipRectMin.x, clipRectMin.y + gridSize * 6.0f }, { clipRectMax.x - gridSize, clipRectMax.y });
}

template<typename T>
static void DrawConfigOption(int32_t rowIndex, float yOffset, const ConfigDef<T>* config)
{
    auto drawList = ImGui::GetForegroundDrawList();
    auto clipRectMin = drawList->GetClipRectMin();
    auto clipRectMax = drawList->GetClipRectMax();

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

    if (g_selectedRowIndex == rowIndex)
    {
        g_selectedItem = config;

        drawList->AddRectFilledMultiColor(min, max, COLOR0, COLOR0, COLOR1, COLOR1);
        DrawTextWithMarquee(g_seuratFont, size, textPos, min, max, IM_COL32_WHITE, configName.c_str(), 250.0f);
    }
    else
    {
        drawList->AddText(g_seuratFont, size, textPos, IM_COL32_WHITE, configName.c_str(), 0, 0.0f, &textClipRect);
    }

    // Right side
    min = { max.x + (clipRectMax.x - max.x - valueWidth) / 2.0f, min.y + (optionHeight - valueHeight) / 2.0f };
    max = { min.x + valueWidth, min.y + valueHeight };

    SetShaderModifier(IMGUI_SHADER_MODIFIER_SCANLINE_BUTTON);

    drawList->AddRectFilledMultiColor(min, max, IM_COL32(0, 130, 0, 223), IM_COL32(0, 130, 0, 178), IM_COL32(0, 130, 0, 223), IM_COL32(0, 130, 0, 178));
    drawList->AddRectFilledMultiColor(min, max, IM_COL32(0, 0, 0, 13), IM_COL32(0, 0, 0, 0), IM_COL32(0, 0, 0, 55), IM_COL32(0, 0, 0, 6));
    drawList->AddRectFilledMultiColor(min, max, IM_COL32(0, 130, 0, 13), IM_COL32(0, 130, 0, 111), IM_COL32(0, 130, 0, 0), IM_COL32(0, 130, 0, 55));

    SetShaderModifier(IMGUI_SHADER_MODIFIER_NONE);

    // Selection triangles
    if (g_selectedItem == config)
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
    }

    auto valueText = config->GetValueLocalised();

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
        IM_COL32_WHITE,
        valueText.data(),
        Scale(2),
        IM_COL32_BLACK);

    ResetGradient();
}

static bool g_upWasHeld;
static bool g_downWasHeld;

static void DrawConfigOptions()
{
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
        DrawConfigOption(rowCount++, yOffset, &Config::UnleashOutOfControlDrain);
        DrawConfigOption(rowCount++, yOffset, &Config::WerehogHubTransformVideo);
        DrawConfigOption(rowCount++, yOffset, &Config::LogoSkip);
        break;
    case 1: // CONTROLS
        DrawConfigOption(rowCount++, yOffset, &Config::CameraXInvert);
        DrawConfigOption(rowCount++, yOffset, &Config::CameraYInvert);
        DrawConfigOption(rowCount++, yOffset, &Config::XButtonHoming);
        DrawConfigOption(rowCount++, yOffset, &Config::UnleashCancel);
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
        DrawConfigOption(rowCount++, yOffset, &Config::ResolutionScale);
        DrawConfigOption(rowCount++, yOffset, &Config::Fullscreen);
        DrawConfigOption(rowCount++, yOffset, &Config::VSync);
        DrawConfigOption(rowCount++, yOffset, &Config::TripleBuffering);
        DrawConfigOption(rowCount++, yOffset, &Config::FPS);
        DrawConfigOption(rowCount++, yOffset, &Config::Brightness);
        DrawConfigOption(rowCount++, yOffset, &Config::AntiAliasing);
        DrawConfigOption(rowCount++, yOffset, &Config::ShadowResolution);
        DrawConfigOption(rowCount++, yOffset, &Config::GITextureFiltering);
        DrawConfigOption(rowCount++, yOffset, &Config::AlphaToCoverage);
        DrawConfigOption(rowCount++, yOffset, &Config::MotionBlur);
        DrawConfigOption(rowCount++, yOffset, &Config::Xbox360ColourCorrection);
        DrawConfigOption(rowCount++, yOffset, &Config::MovieScaleMode);
        DrawConfigOption(rowCount++, yOffset, &Config::UIScaleMode);
        break;
    }

    auto inputState = SWA::CInputState::GetInstance();

    bool upIsHeld = inputState->GetPadState().IsDown(SWA::eKeyState_DpadUp) ||
        inputState->GetPadState().LeftStickVertical > 0.5f;

    bool downIsHeld = inputState->GetPadState().IsDown(SWA::eKeyState_DpadDown) ||
        inputState->GetPadState().LeftStickVertical < -0.5f;

    if (!g_upWasHeld && upIsHeld)
    {
        --g_selectedRowIndex;
        if (g_selectedRowIndex < 0)
            g_selectedRowIndex = rowCount - 1;
    }
    else if (!g_downWasHeld && downIsHeld)
    {
        ++g_selectedRowIndex;
        if (g_selectedRowIndex >= rowCount)
            g_selectedRowIndex = 0;
    }

    g_upWasHeld = upIsHeld;
    g_downWasHeld = downIsHeld;

    auto drawList = ImGui::GetForegroundDrawList();
    auto clipRectMin = drawList->GetClipRectMin();
    auto clipRectMax = drawList->GetClipRectMax();
    int32_t visibleRowCount = int32_t(floor((clipRectMax.y - clipRectMin.y) / optionHeightWithPadding));

    if (g_firstVisibleRowIndex > g_selectedRowIndex)
        g_firstVisibleRowIndex = g_selectedRowIndex;

    if (g_firstVisibleRowIndex + visibleRowCount - 1 < g_selectedRowIndex)
        g_firstVisibleRowIndex = std::max(0, g_selectedRowIndex - visibleRowCount + 1);

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

void DrawSettingsPanel()
{
    auto drawList = ImGui::GetForegroundDrawList();

    ImVec2 settingsMin = { Scale(AlignToNextGrid(COMMON_PADDING_POS_X)), Scale(AlignToNextGrid(COMMON_PADDING_POS_Y)) };
    ImVec2 settingsMax = { Scale(AlignToNextGrid(INFO_CONTAINER_POS_X - COMMON_PADDING_POS_X)), Scale(AlignToNextGrid(720.0f - COMMON_PADDING_POS_Y)) };

    DrawContainer(settingsMin, settingsMax);
    DrawCategories();
    DrawConfigOptions();

    // Pop clip rect from DrawContainer
    drawList->PopClipRect();
}

void DrawInfoPanel()
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
            auto resScale = *(float*)g_selectedItem->GetValue();

            std::snprintf(buf, sizeof(buf), desc.c_str(),
                (int)((float)Window::s_width * resScale),
                (int)((float)Window::s_height * resScale));

            desc = buf;
        }

        auto size = Scale(26.0f);
        auto textSize = g_seuratFont->CalcTextSizeA(size, FLT_MAX, 0.0f, desc.c_str());

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
    if (!s_isVisible)
        return;

    g_callbackDataIndex = 0;
    
    auto& res = ImGui::GetIO().DisplaySize;
    auto drawList = ImGui::GetForegroundDrawList();
    
    if (s_isDimBackground)
        drawList->AddRectFilled({ 0.0f, 0.0f }, res, IM_COL32(0, 0, 0, 127));
    
    DrawScanlineBars();
    DrawSettingsPanel();
    DrawInfoPanel();
}

void OptionsMenu::Open(bool stage)
{
    s_isVisible = true;
    s_isDimBackground = stage;
    g_upWasHeld = false;
    g_downWasHeld = false;

    *(bool*)g_memory.Translate(0x8328BB26) = false;

    // TODO: animate Miles Electric in if we're in a stage.
}

void OptionsMenu::Close(bool stage)
{
    s_isVisible = false;
    s_isDimBackground = stage;

    *(bool*)g_memory.Translate(0x8328BB26) = true;

    // TODO: animate Miles Electric out if we're in a stage.
}
