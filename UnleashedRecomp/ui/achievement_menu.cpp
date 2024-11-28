#include "achievement_menu.h"
#include "imgui_utils.h"
#include <xdbf_wrapper.h>
#include <api/SWA.h>
#include <gpu/video.h>
#include <locale/locale.h>
#include <user/achievement_data.h>
#include <user/config.h>
#include <app.h>
#include <exports.h>

AchievementMenu m_achievementMenu;

static std::vector<Achievement> g_achievements;
static std::unordered_map<uint16_t, GuestTexture*> g_achievementTextures;

static ImFont* g_fntSeurat;
static ImFont* g_fntNewRodin;

static int g_firstVisibleRowIndex;
static int g_prevSelectedRowIndex;
static int g_selectedRowIndex;
static double g_rowSelectionTime;

static bool g_upWasHeld;
static bool g_downWasHeld;

static void ResetSelection()
{
    g_firstVisibleRowIndex = 0;
    g_selectedRowIndex = 0;
    g_prevSelectedRowIndex = 0;
    g_rowSelectionTime = ImGui::GetTime();
    g_upWasHeld = false;
    g_downWasHeld = false;
}

void AchievementMenu::Init()
{
    auto& io = ImGui::GetIO();

    constexpr float FONT_SCALE = 2.0f;

    g_fntSeurat = io.Fonts->AddFontFromFileTTF("FOT-SeuratPro-M.otf", 24.0f * FONT_SCALE);
    g_fntNewRodin = io.Fonts->AddFontFromFileTTF("FOT-NewRodinPro-UB.otf", 20.0f * FONT_SCALE);

    g_achievements = g_xdbf.GetAchievements((EXDBFLanguage)Config::Language.Value);

    for (auto& achievement : g_achievements)
    {
        auto texture = LoadTexture((uint8_t*)achievement.pImageBuffer, achievement.ImageBufferSize);
        g_achievementTextures[achievement.ID] = texture.release();
    }
}

static void DrawContainer(ImVec2 min, ImVec2 max, ImU32 gradientTop, ImU32 gradientBottom, float cornerRadius = 25.0f)
{
    auto drawList = ImGui::GetForegroundDrawList();

    ImVec2 v1 = { min.x, min.y + cornerRadius };
    ImVec2 v2 = { min.x + cornerRadius, min.y };
    ImVec2 v3 = { max.x, min.y };
    ImVec2 v4 = { max.x, min.y + cornerRadius };
    ImVec2 v5 = { max.x, max.y - cornerRadius };
    ImVec2 v6 = { max.x - cornerRadius, max.y };
    ImVec2 v7 = { min.x, max.y };
    ImVec2 v8 = { min.x, max.y - cornerRadius };
    ImVec2 vertices[] = { v1, v2, v3, v4, v5, v6, v7, v8 };

    // TODO: add a drop shadow.

    SetGradient(min, max, gradientTop, gradientBottom);
    drawList->AddConvexPolyFilled(vertices, IM_ARRAYSIZE(vertices), IM_COL32(255, 255, 255, 255));
    ResetGradient();

    drawList->AddPolyline(vertices, IM_ARRAYSIZE(vertices), IM_COL32(247, 247, 247, 255), true, Scale(2.5f));

    for (int i = 0; i < IM_ARRAYSIZE(vertices); i++)
    {
        vertices[i].x -= 0.4f;
        vertices[i].y -= 0.2f;
    }

    auto colLineTop = IM_COL32(165, 170, 165, 230);
    auto colLineBottom = IM_COL32(190, 190, 190, 230);
    auto lineThickness = Scale(1.0f);

    // Top left corner bottom to top left corner top.
    drawList->AddLine(vertices[0], vertices[1], colLineTop, lineThickness * 0.5f);

    // Top left corner bottom to bottom left.
    drawList->AddRectFilledMultiColor({ vertices[0].x - 0.2f, vertices[0].y }, { vertices[6].x + lineThickness - 0.2f, vertices[6].y }, colLineTop, colLineTop, colLineBottom, colLineBottom);

    // Top left corner top to top right.
    drawList->AddLine(vertices[1], vertices[2], colLineTop, lineThickness);

    drawList->PushClipRect({ min.x, min.y + 20.0f }, { max.x, max.y - 5.0f });
}

static void DrawSelectionContainer(ImVec2 min, ImVec2 max)
{
    auto drawList = ImGui::GetForegroundDrawList();

    auto cornerRadius = Scale(10.0f);
    ImVec2 v1 = { min.x, min.y + cornerRadius };
    ImVec2 v2 = { min.x + cornerRadius, min.y };
    ImVec2 v3 = { max.x, min.y };
    ImVec2 v4 = { max.x, min.y + cornerRadius };
    ImVec2 v5 = { max.x, max.y - cornerRadius };
    ImVec2 v6 = { max.x - cornerRadius, max.y };
    ImVec2 v7 = { min.x, max.y };
    ImVec2 v8 = { min.x, max.y - cornerRadius };
    ImVec2 vertices[] = { v1, v2, v3, v4, v5, v6, v7, v8 };

    SetGradient(min, max, IM_COL32(255, 246, 0, 129), IM_COL32(255, 194, 0, 118));
    drawList->AddConvexPolyFilled(vertices, IM_ARRAYSIZE(vertices), IM_COL32(255, 255, 255, 255));
    ResetGradient();
}

static void DrawHeaderContainer(const char* text)
{
    auto drawList = ImGui::GetForegroundDrawList();

    ImVec2 min = { Scale(256.0f), Scale(138.0f) };
    ImVec2 max = { Scale(556.0f), Scale(185.0f) };

    DrawContainer(min, max, IM_COL32(140, 142, 140, 201), IM_COL32(66, 65, 66, 234), Scale(23.0f));
    drawList->PopClipRect();

    auto textSize = g_fntNewRodin->CalcTextSizeA(Scale(26.0f), FLT_MAX, 0.0f, text);

    // TODO: skew this text and apply bevel.
    DrawTextWithOutline<int>(g_fntNewRodin, Scale(26.0f), { min.x + Scale(20.0f), min.y + textSize.y / 2.0f - 3.0f }, IM_COL32(255, 255, 255, 255), text, Scale(3), IM_COL32(0, 0, 0, 255));
}

static void DrawAchievement(int rowIndex, float yOffset, Achievement& achievement, bool isUnlocked)
{
    auto drawList = ImGui::GetForegroundDrawList();

    auto clipRectMin = drawList->GetClipRectMin();
    auto clipRectMax = drawList->GetClipRectMax();

    auto itemWidth = Scale(708.0f);
    auto itemHeight = Scale(94.0f);
    auto itemMarginX = Scale(13.0f);
    auto imageMarginX = Scale(25.0f);
    auto imageMarginY = Scale(18.0f);
    auto imageSize = Scale(60.0f);

    ImVec2 min = { itemMarginX + clipRectMin.x, clipRectMin.y + itemHeight * rowIndex + yOffset };
    ImVec2 max = { itemMarginX + min.x + itemWidth, min.y + itemHeight };

    auto icon = g_achievementTextures[achievement.ID];
    auto isSelected = rowIndex == g_selectedRowIndex;

    if (isSelected)
        DrawSelectionContainer(min, max);

    auto alpha = isUnlocked ? 255 : 127;
    auto desc = isUnlocked ? achievement.UnlockedDesc.c_str() : achievement.LockedDesc.c_str();
    auto fontSize = Scale(24.0f);
    auto textSize = g_fntSeurat->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, desc);
    auto textX = min.x + imageMarginX + imageSize + itemMarginX * 2.0f;
    auto textMarqueeX = min.x + imageMarginX + imageSize;
    auto titleTextY = Scale(20.0f);
    auto descTextY = Scale(52.0f);
    auto cmnShadowOffset = Scale(2.0f);
    auto cmnShadowScale = Scale(0.4f);

    // Draw achievement icon.
    // TODO: make image greyscale if locked.
    drawList->AddImage(icon, { min.x + imageMarginX, min.y + imageMarginY }, { min.x + imageMarginX + imageSize, min.y + imageMarginY + imageSize }, { 0, 0 }, { 1, 1 }, IM_COL32(255, 255, 255, alpha));

    drawList->PushClipRect(min, max, true);

    // Draw achievement name.
    DrawTextWithShadow(g_fntSeurat, fontSize, { textX, min.y + titleTextY }, IM_COL32(252, 243, 5, alpha), achievement.Name.c_str(), cmnShadowOffset, cmnShadowScale, IM_COL32(0, 0, 0, alpha));

    if (isSelected && textX + textSize.x >= max.x)
    {
        // Draw achievement description with marquee.
        DrawTextWithMarqueeShadow(g_fntSeurat, fontSize, { textX, min.y + descTextY }, { textMarqueeX, min.y }, max, IM_COL32(255, 255, 255, alpha), desc, g_rowSelectionTime, 0.9, 250.0, cmnShadowOffset, cmnShadowScale);
    }
    else
    {
        // Draw achievement description.
        DrawTextWithShadow(g_fntSeurat, fontSize, { textX, min.y + descTextY }, IM_COL32(255, 255, 255, alpha), desc, cmnShadowOffset, cmnShadowScale, IM_COL32(0, 0, 0, alpha));
    }

    drawList->PopClipRect();
}

static void DrawContentContainer()
{
    auto drawList = ImGui::GetForegroundDrawList();

    ImVec2 min = { Scale(256.0f), Scale(192.0f) };
    ImVec2 max = { Scale(1026.0f), Scale(601.0f) };

    DrawContainer(min, max, IM_COL32(197, 194, 197, 200), IM_COL32(115, 113, 115, 236), Scale(25.0f));

    auto clipRectMin = drawList->GetClipRectMin();
    auto clipRectMax = drawList->GetClipRectMax();

    auto itemHeight = Scale(94.0f);
    auto yOffset = -g_firstVisibleRowIndex * itemHeight + Scale(2.0f);
    auto rowCount = 0;

    // Draw separators.
    for (int i = 1; i <= 3; i++)
    {
        auto lineMarginLeft = Scale(31.0f);
        auto lineMarginRight = Scale(46.0f);
        auto lineMarginY = Scale(2.0f);

        ImVec2 lineMin = { clipRectMin.x + lineMarginLeft, clipRectMin.y + itemHeight * i + lineMarginY };
        ImVec2 lineMax = { clipRectMax.x - lineMarginRight, clipRectMin.y + itemHeight * i + lineMarginY };

        drawList->AddLine(lineMin, lineMax, IM_COL32(163, 163, 163, 255));
        drawList->AddLine({ lineMin.x, lineMin.y + Scale(1.0f) }, { lineMax.x, lineMax.y + Scale(1.0f) }, IM_COL32(143, 148, 143, 255));
    }

    for (auto achievement : g_achievements)
    {
        if (AchievementData::IsUnlocked(achievement.ID))
            DrawAchievement(rowCount++, yOffset, achievement, true);
    }

    for (auto achievement : g_achievements)
    {
        if (!AchievementData::IsUnlocked(achievement.ID))
            DrawAchievement(rowCount++, yOffset, achievement, false);
    }

    auto inputState = SWA::CInputState::GetInstance();

    bool upIsHeld = inputState->GetPadState().IsDown(SWA::eKeyState_DpadUp) ||
        inputState->GetPadState().LeftStickVertical > 0.5f;

    bool downIsHeld = inputState->GetPadState().IsDown(SWA::eKeyState_DpadDown) ||
        inputState->GetPadState().LeftStickVertical < -0.5f;

    bool scrollUp = !g_upWasHeld && upIsHeld;
    bool scrollDown = !g_downWasHeld && downIsHeld;

    int prevSelectedRowIndex = g_selectedRowIndex;

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
        Game_PlaySound("sys_actstg_pausecursor");
    }

    g_upWasHeld = upIsHeld;
    g_downWasHeld = downIsHeld;

    int visibleRowCount = int(floor((clipRectMax.y - clipRectMin.y) / itemHeight));

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

    // Pop clip rect from DrawContentContainer
    drawList->PopClipRect();

    // Draw scroll bar
    if (rowCount > visibleRowCount)
    {
        float cornerRadius = Scale(25.0f);
        float totalHeight = (clipRectMax.y - clipRectMin.y - cornerRadius) - Scale(3.0f);
        float heightRatio = float(visibleRowCount) / float(rowCount);
        float offsetRatio = float(g_firstVisibleRowIndex) / float(rowCount);
        float offsetX = clipRectMax.x - Scale(31.0f);
        float offsetY = offsetRatio * totalHeight + clipRectMin.y + Scale(4.0f);
        float lineThickness = Scale(1.0f);
        float innerMarginX = Scale(2.0f);
        float outerMarginX = Scale(16.0f);

        // Outline
        drawList->AddRect
        (
            { offsetX - lineThickness, clipRectMin.y - lineThickness },
            { clipRectMax.x - outerMarginX + lineThickness, max.y - cornerRadius + lineThickness },
            IM_COL32(255, 255, 255, 155),
            Scale(0.5f)
        );

        // Background
        drawList->AddRectFilledMultiColor
        (
            { offsetX, clipRectMin.y },
            { clipRectMax.x - outerMarginX, max.y - cornerRadius },
            IM_COL32(82, 85, 82, 186),
            IM_COL32(82, 85, 82, 186),
            IM_COL32(74, 73, 74, 185),
            IM_COL32(74, 73, 74, 185)
        );

        // Scroll Bar Outline
        drawList->AddRectFilledMultiColor
        (
            { offsetX + innerMarginX, offsetY - lineThickness },
            { clipRectMax.x - outerMarginX - innerMarginX, offsetY + lineThickness + totalHeight * heightRatio},
            IM_COL32(185, 185, 185, 255),
            IM_COL32(185, 185, 185, 255),
            IM_COL32(172, 172, 172, 255),
            IM_COL32(172, 172, 172, 255)
        );

        // Scroll Bar
        drawList->AddRectFilled
        (
            { offsetX + innerMarginX + lineThickness, offsetY },
            { clipRectMax.x - outerMarginX - innerMarginX - lineThickness, offsetY + totalHeight * heightRatio },
            IM_COL32(255, 255, 255, 255)
        );
    }
}

void AchievementMenu::Draw()
{
    if (!s_isVisible)
        return;

    DrawHeaderContainer(Localise("Achievements_Name_Uppercase").c_str());
    DrawContentContainer();
}

void AchievementMenu::Open()
{
    s_isVisible = true;

    ResetSelection();
    Game_PlaySound("sys_actstg_pausewinopen");
}

void AchievementMenu::Close()
{
    s_isVisible = false;

    Game_PlaySound("sys_actstg_pausewinclose");
    Game_PlaySound("sys_actstg_pausecansel");
}
