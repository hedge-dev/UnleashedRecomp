#include "options_menu.h"
#include "window.h"
#include <kernel/memory.h>
#include <gpu/imgui_common.h>

static ImFont* g_seuratFont;
static ImFont* g_dfsogeistdFont;
static ImFont* g_newRodinFont;

void OptionsMenu::Init()
{
    auto& io = ImGui::GetIO();
    g_seuratFont = io.Fonts->AddFontFromFileTTF("FOT-SeuratPro-M.otf", 26.0f);
    g_dfsogeistdFont = io.Fonts->AddFontFromFileTTF("DFSoGeiStd-W7.otf", 48.0f);
    g_newRodinFont = io.Fonts->AddFontFromFileTTF("FOT-NewRodinPro-DB.otf", 20.0f);
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

    SetShaderModifier(IMGUI_SHADER_MODIFIER_SCANLINE);

    // Top bar
    drawList->AddRectFilledMultiColor(
        { 0.0f, 0.0f },
        { res.x, height },
        FADE_COLOR0,
        FADE_COLOR0,
        FADE_COLOR1,
        FADE_COLOR1);

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
        FADE_COLOR0,
        FADE_COLOR0,
        FADE_COLOR1,
        FADE_COLOR1);

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

static uint32_t g_categoryIndex;

static void DrawCategories()
{
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

        if (g_categoryIndex == i || ImGui::IsMouseHoveringRect(min, max, false))
        {
            SetShaderModifier(IMGUI_SHADER_MODIFIER_SCANLINE_BUTTON);

            drawList->AddRectFilledMultiColor(min, max, IM_COL32(0, 130, 0, 223), IM_COL32(0, 130, 0, 178), IM_COL32(0, 130, 0, 223), IM_COL32(0, 130, 0, 178));
            drawList->AddRectFilledMultiColor(min, max, IM_COL32(0, 0, 0, 13), IM_COL32(0, 0, 0, 0), IM_COL32(0, 0, 0, 55), IM_COL32(0, 0, 0, 6));
            drawList->AddRectFilledMultiColor(min, max, IM_COL32(0, 130, 0, 13), IM_COL32(0, 130, 0, 111), IM_COL32(0, 130, 0, 0), IM_COL32(0, 130, 0, 55));

            SetShaderModifier(IMGUI_SHADER_MODIFIER_NONE);

            if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
                g_categoryIndex = i;
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

    drawList->PushClipRect({ clipRectMin.x, clipRectMin.y + gridSize * 6.0f }, clipRectMax);
}

static void DrawConfigOptions()
{
    auto drawList = ImGui::GetForegroundDrawList();
    auto clipRectMin = drawList->GetClipRectMin();
    auto clipRectMax = drawList->GetClipRectMax();

    constexpr ImU32 COLOR0 = IM_COL32(0xE2, 0x71, 0x22, 0x80);
    constexpr ImU32 COLOR1 = IM_COL32(0x92, 0xFF, 0x31, 0x80);

    float gridSize = Scale(GRID_SIZE);
    float optionWidth = gridSize * 56.0f;
    float optionHeight = gridSize * 5.5f;
    float optionPadding = gridSize * 0.5f;
    float valueWidth = gridSize * 24.0f;
    float valueHeight = gridSize * 3.0f;
    int32_t currentRow = 0;

    for (auto& config : Config::Definitions)
    {
        if (_stricmp(config->GetSection().c_str(), CATEGORIES[g_categoryIndex]) != 0)
            continue;

        // Left side
        ImVec2 min = { clipRectMin.x, clipRectMin.y + (optionHeight + optionPadding) * currentRow };
        ImVec2 max = { min.x + optionWidth, min.y + optionHeight };

        if (ImGui::IsMouseHoveringRect(min, max, false))
            drawList->AddRectFilledMultiColor(min, max, COLOR0, COLOR0, COLOR1, COLOR1);

        float size = Scale(26.0f);
        auto textSize = g_seuratFont->CalcTextSizeA(size, FLT_MAX, 0.0f, config->GetName().c_str());

        drawList->AddText(g_seuratFont, size, { min.x + GRID_SIZE, min.y + (optionHeight - textSize.y) / 2.0f }, IM_COL32_WHITE, config->GetName().c_str());

        // Right side
        min = { max.x + (clipRectMax.x - max.x - valueWidth) / 2.0f, min.y + (optionHeight - valueHeight) / 2.0f };
        max = { min.x + valueWidth, min.y + valueHeight };

        SetShaderModifier(IMGUI_SHADER_MODIFIER_SCANLINE_BUTTON);

        drawList->AddRectFilledMultiColor(min, max, IM_COL32(0, 130, 0, 223), IM_COL32(0, 130, 0, 178), IM_COL32(0, 130, 0, 223), IM_COL32(0, 130, 0, 178));
        drawList->AddRectFilledMultiColor(min, max, IM_COL32(0, 0, 0, 13), IM_COL32(0, 0, 0, 0), IM_COL32(0, 0, 0, 55), IM_COL32(0, 0, 0, 6));
        drawList->AddRectFilledMultiColor(min, max, IM_COL32(0, 130, 0, 13), IM_COL32(0, 130, 0, 111), IM_COL32(0, 130, 0, 0), IM_COL32(0, 130, 0, 55));

        SetShaderModifier(IMGUI_SHADER_MODIFIER_NONE);

        auto valueText = config->ToString();
        std::transform(valueText.begin(), valueText.end(), valueText.begin(), toupper);

        size = Scale(20.0f);
        textSize = g_newRodinFont->CalcTextSizeA(size, FLT_MAX, 0.0f, valueText.c_str());

        min.x += ((max.x - min.x) - textSize.x) / 2.0f;
        min.y += ((max.y - min.y) - textSize.y) / 2.0f;

        SetGradient(
            min,
            {min.x + textSize.x, min.y + textSize.y},
            IM_COL32(192, 255, 0, 255),
            IM_COL32(128, 170, 0, 255)
        );

        DrawTextWithOutline(
            g_newRodinFont,
            size,
            min,
            IM_COL32_WHITE,
            valueText.c_str(),
            Scale(2),
            IM_COL32_BLACK);

        ResetGradient();

        ++currentRow;
    }
}

void OptionsMenu::Draw()
{
    g_callbackDataIndex = 0;

    auto& res = ImGui::GetIO().DisplaySize;
    auto drawList = ImGui::GetForegroundDrawList();

    //drawList->AddRectFilled({ 0.0f, 0.0f }, res, IM_COL32(0, 0, 0, 223));

    //*(bool*)g_memory.Translate(0x8328BB26) = false;

    DrawScanlineBars();

    constexpr float CONTAINER_POS_X = 236.0f;
    constexpr float CONTAINER_POS_Y = 118.0f;
    
    ImVec2 min = { Scale(AlignToNextGrid(CONTAINER_POS_X)), Scale(AlignToNextGrid(CONTAINER_POS_Y)) };
    ImVec2 max = { Scale(AlignToNextGrid(1280.0f - CONTAINER_POS_X)), Scale(AlignToNextGrid(720.0f - CONTAINER_POS_Y)) };

    DrawContainer(min, max);

    DrawCategories();

    DrawConfigOptions();

    // Pop clip rect from DrawCategories
    drawList->PopClipRect();

    // Pop clip rect from DrawContainer
    drawList->PopClipRect();
}
