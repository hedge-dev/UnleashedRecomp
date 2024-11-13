#include "options_menu.h"
#include "window.h"
#include <kernel/memory.h>

static ImFont* g_seuratFont;
static ImFont* g_dfsogeistdFont;
static ImFont* g_newRodinFont;

void OptionsMenu::Init()
{
    auto& io = ImGui::GetIO();
    g_seuratFont = io.Fonts->AddFontFromFileTTF("FOT-SeuratPro-M.otf", 26.0f);
    g_dfsogeistdFont = io.Fonts->AddFontFromFileTTF("DFSoGeiStd-W7.otf", 48.0f);
    g_newRodinFont = io.Fonts->AddFontFromFileTTF("FOT-NewRodinPro-DB.otf", 16.0f);
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
    constexpr uint32_t COLOR0 = IM_COL32(58, 71, 15, 0);
    constexpr uint32_t COLOR1 = IM_COL32(58, 71, 15, 255);
    constexpr uint32_t OUTLINE_COLOR = IM_COL32(115, 178, 104, 255);

    float height = Scale(105.0f);

    auto& res = ImGui::GetIO().DisplaySize;
    auto drawList = ImGui::GetForegroundDrawList();

    // Top bar
    drawList->AddRectFilledMultiColor(
        { 0.0f, 0.0f },
        { res.x, height },
        COLOR0, 
        COLOR0, 
        COLOR1,
        COLOR1);

    drawList->AddLine(
        { 0.0f, height }, 
        { res.x, height },
        OUTLINE_COLOR);

    // Options text
    DrawTextWithOutline(g_dfsogeistdFont, Scale(48.0f), { Scale(122.0f), Scale(56.0f) }, IM_COL32(255, 195, 0, 255), "OPTIONS", Scale(4), IM_COL32_BLACK);

    // Bottom bar
    drawList->AddRectFilledMultiColor(
        { res.x, res.y }, 
        { 0.0f, res.y - height },
        COLOR0, 
        COLOR0,
        COLOR1,
        COLOR1);

    drawList->AddLine(
        { 0.0f, res.y - height },
        { res.x, res.y - height },
        OUTLINE_COLOR);
}

static constexpr size_t GRID_SIZE = 8;

static void DrawContainer(const ImVec2& min, const ImVec2& max)
{
    auto& res = ImGui::GetIO().DisplaySize;
    auto drawList = ImGui::GetForegroundDrawList();

    constexpr uint32_t COLOR = IM_COL32(0, 49, 0, 255);
    constexpr uint32_t INNER_COLOR = IM_COL32(0, 33, 0, 255);
    constexpr uint32_t LINE_COLOR = IM_COL32(0, 89, 0, 255);

    float gridSize = Scale(GRID_SIZE);

    drawList->AddRectFilled(min, max, COLOR); // Container
    drawList->AddRectFilled({ min.x + gridSize, min.y + gridSize }, {max.x - gridSize, max.y - gridSize}, INNER_COLOR); // Inner container

    // Top line
    drawList->AddLine({ min.x + gridSize, min.y + gridSize }, { min.x + gridSize, min.y + gridSize * 2.0f }, LINE_COLOR, Scale(1)); // Vertical left
    drawList->AddLine({ min.x + gridSize, min.y + gridSize }, { max.x - gridSize, min.y + gridSize }, LINE_COLOR, Scale(1)); // Horizontal
    drawList->AddLine({ max.x - gridSize, min.y + gridSize }, { max.x - gridSize, min.y + gridSize * 2.0f }, LINE_COLOR, Scale(1)); // Vertical right

    // Bottom line
    drawList->AddLine({ min.x + gridSize, max.y - gridSize }, { min.x + gridSize, max.y - gridSize * 2.0f }, LINE_COLOR, Scale(1)); // Vertical left
    drawList->AddLine({ min.x + gridSize, max.y - gridSize }, { max.x - gridSize, max.y - gridSize }, LINE_COLOR, Scale(1)); // Horizontal
    drawList->AddLine({ max.x - gridSize, max.y - gridSize }, { max.x - gridSize, max.y - gridSize * 2.0f }, LINE_COLOR, Scale(1)); // Vertical right

    // The draw area
    drawList->PushClipRect({ min.x + gridSize * 2.0f, min.y + gridSize * 2.0f }, { max.x - gridSize * 2.0f, max.y - gridSize * 2.0f });
}

static constexpr const char* CATEGORIES[] =
{
    "SYSTEM",
    "CONTROLS",
    "AUDIO",
    "VIDEO"
};

static uint32_t g_categoryIndex;

static float ComputeSizeWithPadding(float width, float padding, int32_t elementSize)
{
    return (width - padding * (elementSize - 1)) / elementSize;
}

static void DrawCategories()
{
    auto drawList = ImGui::GetForegroundDrawList();
    auto clipRectMin = drawList->GetClipRectMin();
    auto clipRectMax = drawList->GetClipRectMax();

    float gridSize = Scale(GRID_SIZE);
    float tabPadding = gridSize * 2.0f;
    float tabWidth = ComputeSizeWithPadding(clipRectMax.x - clipRectMin.x, tabPadding, std::size(CATEGORIES));
    float tabWidthWithPadding = tabWidth + tabPadding;
    float tabHeight = gridSize * 4.5f;

    for (size_t i = 0; i < std::size(CATEGORIES); i++)
    {
        ImVec2 min = { clipRectMin.x + tabWidthWithPadding * i, clipRectMin.y };
        ImVec2 max = { min.x + tabWidth, min.y + tabHeight };

        if (g_categoryIndex == i || ImGui::IsMouseHoveringRect(min, max, false))
        {
            drawList->AddRectFilled(min, max, IM_COL32(0, 96, 0, 255));

            if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
                g_categoryIndex = i;
        }

        float size = Scale(24.0f);
        auto textSize = g_dfsogeistdFont->CalcTextSizeA(size, FLT_MAX, 0.0f, CATEGORIES[i]);

        DrawTextWithOutline(
            g_dfsogeistdFont, 
            size,
            { min.x + (tabWidth - textSize.x) / 2.0f, min.y + (tabHeight - textSize.y) / 2.0f },
            IM_COL32(150, 230, 30, 255), 
            CATEGORIES[i], 
            Scale(3),
            IM_COL32_BLACK);
    }

    drawList->PushClipRect({ clipRectMin.x, clipRectMin.y + gridSize * 6.0f }, clipRectMax);
}

static void DrawConfigOptions()
{
    auto drawList = ImGui::GetForegroundDrawList();
    auto clipRectMin = drawList->GetClipRectMin();
    auto clipRectMax = drawList->GetClipRectMax();

    constexpr ImU32 COLOR0 = IM_COL32(113, 57, 17, 255);
    constexpr ImU32 COLOR1 = IM_COL32(74, 138, 25, 255);

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

        drawList->AddRectFilled(min, max, IM_COL32(0, 96, 0, 255));

        auto valueText = config->ToString();
        std::transform(valueText.begin(), valueText.end(), valueText.begin(), toupper);

        size = Scale(16.0f);
        textSize = g_newRodinFont->CalcTextSizeA(size, FLT_MAX, 0.0f, valueText.c_str());

        DrawTextWithOutline(
            g_newRodinFont,
            size,
            {min.x + ((max.x - min.x) - textSize.x) / 2.0f, min.y + ((max.y - min.y) - textSize.y) / 2.0f },
            IM_COL32(167, 234, 67, 255),
            valueText.c_str(),
            Scale(2),
            IM_COL32_BLACK);

        ++currentRow;
    }
}

void OptionsMenu::Draw()
{
    auto& res = ImGui::GetIO().DisplaySize;
    auto drawList = ImGui::GetForegroundDrawList();

    *(bool*)g_memory.Translate(0x8328BB26) = false;

    DrawScanlineBars();

    float containerPosX = Scale(/*276.0f*/250.0f);
    float containerPosY = Scale(118.0f);
    
    ImVec2 min = { containerPosX, containerPosY };
    ImVec2 max = { res.x - containerPosX, res.y - containerPosY };

    DrawContainer(min, max);

    DrawCategories();

    DrawConfigOptions();

    // Pop clip rect from DrawCategories
    drawList->PopClipRect();

    // Pop clip rect from DrawContainer
    drawList->PopClipRect();
}
