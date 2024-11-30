#include "achievement_menu.h"
#include "imgui_utils.h"
#include <api/SWA.h>
#include <gpu/video.h>
#include <kernel/xdbf.h>
#include <locale/locale.h>
#include <user/achievement_data.h>
#include <user/config.h>
#include <app.h>
#include <exports.h>

static std::vector<Achievement> m_achievements;

static ImFont* m_fntSeurat;
static ImFont* m_fntNewRodin;

static int m_firstVisibleRowIndex;
static int m_prevSelectedRowIndex;
static int m_selectedRowIndex;
static double m_rowSelectionTime;

static bool m_upWasHeld;
static bool m_downWasHeld;

static void ResetSelection()
{
    m_firstVisibleRowIndex = 0;
    m_selectedRowIndex = 0;
    m_prevSelectedRowIndex = 0;
    m_rowSelectionTime = ImGui::GetTime();
    m_upWasHeld = false;
    m_downWasHeld = false;
}

static void DrawContainer(ImVec2 min, ImVec2 max, ImU32 gradientTop, ImU32 gradientBottom, float cornerRadius = 25)
{
    auto drawList = ImGui::GetForegroundDrawList();
    auto vertices = GetPauseContainerVertices(min, max, cornerRadius);

    // TODO: add a drop shadow.

    SetGradient(min, max, gradientTop, gradientBottom);
    drawList->AddConvexPolyFilled(vertices.data(), vertices.size(), IM_COL32(255, 255, 255, 255));
    ResetGradient();

    drawList->AddPolyline(vertices.data(), vertices.size(), IM_COL32(247, 247, 247, 255), true, Scale(2.5f));

    for (int i = 0; i < vertices.size(); i++)
    {
        vertices[i].x -= Scale(0.4f);
        vertices[i].y -= Scale(0.2f);
    }

    auto colLineTop = IM_COL32(165, 170, 165, 230);
    auto colLineBottom = IM_COL32(190, 190, 190, 230);
    auto lineThickness = Scale(1);

    // Top left corner bottom to top left corner top.
    drawList->AddLine(vertices[0], vertices[1], colLineTop, lineThickness * Scale(0.5f));

    // Top left corner bottom to bottom left.
    drawList->AddRectFilledMultiColor
    (
        { /* X */ vertices[0].x - Scale(0.2f), /* Y */ vertices[0].y },
        { /* X */ vertices[6].x + lineThickness - Scale(0.2f), /* Y */ vertices[6].y },
        colLineTop,
        colLineTop,
        colLineBottom,
        colLineBottom
    );

    // Top left corner top to top right.
    drawList->AddLine(vertices[1], vertices[2], colLineTop, lineThickness);

    drawList->PushClipRect({ min.x, min.y + Scale(20) }, { max.x, max.y - Scale(5) });
}

static void DrawSelectionContainer(ImVec2 min, ImVec2 max)
{
    auto drawList = ImGui::GetForegroundDrawList();
    auto vertices = GetPauseContainerVertices(min, max, 10);

    SetGradient(min, max, IM_COL32(255, 246, 0, 129), IM_COL32(255, 194, 0, 118));
    drawList->AddConvexPolyFilled(vertices.data(), vertices.size(), IM_COL32(255, 255, 255, 255));
    ResetGradient();
}

static void DrawHeaderContainer(const char* text)
{
    auto drawList = ImGui::GetForegroundDrawList();
    auto fontSize = Scale(26);
    auto textSize = m_fntNewRodin->CalcTextSizeA(fontSize, FLT_MAX, 0, text);
    auto cornerRadius = 23;
    auto textMarginX = Scale(16) + (Scale(cornerRadius) / 2);

    ImVec2 min = { Scale(256), Scale(138) };
    ImVec2 max = { min.x + textMarginX * 2 + textSize.x, Scale(185) };

    DrawContainer(min, max, IM_COL32(140, 142, 140, 201), IM_COL32(66, 65, 66, 234), cornerRadius);
    drawList->PopClipRect();

    // TODO: skew this text and apply bevel.
    DrawTextWithOutline<int>
    (
        m_fntNewRodin,
        fontSize,
        { /* X */ min.x + textMarginX, /* Y */ min.y + textSize.y / Scale(2) + Scale(2.5f) /* 2.5 = container outline thickness */ },
        IM_COL32(255, 255, 255, 255),
        text,
        3,
        IM_COL32(0, 0, 0, 255)
    );
}

static void DrawAchievement(int rowIndex, float yOffset, Achievement& achievement, bool isUnlocked)
{
    auto drawList = ImGui::GetForegroundDrawList();

    auto clipRectMin = drawList->GetClipRectMin();
    auto clipRectMax = drawList->GetClipRectMax();

    auto itemWidth = Scale(708);
    auto itemHeight = Scale(94);
    auto itemMarginX = Scale(13);
    auto imageMarginX = Scale(25);
    auto imageMarginY = Scale(18);
    auto imageSize = Scale(60);

    ImVec2 min = { itemMarginX + clipRectMin.x, clipRectMin.y + itemHeight * rowIndex + yOffset };
    ImVec2 max = { itemMarginX + min.x + itemWidth, min.y + itemHeight };

    auto icon = g_xdbfTextureCache[achievement.ID];
    auto isSelected = rowIndex == m_selectedRowIndex;

    if (isSelected)
        DrawSelectionContainer(min, max);

    auto desc = isUnlocked ? achievement.UnlockedDesc.c_str() : achievement.LockedDesc.c_str();
    auto fontSize = Scale(24);
    auto textSize = m_fntSeurat->CalcTextSizeA(fontSize, FLT_MAX, 0, desc);
    auto textX = min.x + imageMarginX + imageSize + itemMarginX * 2;
    auto textMarqueeX = min.x + imageMarginX + imageSize;
    auto titleTextY = Scale(20);
    auto descTextY = Scale(52);

    // Draw achievement icon.
    // TODO: make icon greyscale if locked?
    drawList->AddImage
    (
        icon,
        { /* X */ min.x + imageMarginX, /* Y */ min.y + imageMarginY },
        { /* X */ min.x + imageMarginX + imageSize, /* Y */ min.y + imageMarginY + imageSize },
        { /* U */ 0, /* V */ 0 },
        { /* U */ 1, /* V */ 1 },
        IM_COL32(255, 255, 255, 255 * (isUnlocked ? 1 : 0.5f))
    );

    drawList->PushClipRect(min, max, true);

    auto colLockedText = IM_COL32(60, 60, 60, 29);

    auto colTextShadow = isUnlocked
        ? IM_COL32(0, 0, 0, 255)
        : IM_COL32(60, 60, 60, 28);

    auto shadowOffset = isUnlocked ? 2 : 1;

    // Draw achievement name.
    DrawTextWithShadow
    (
        m_fntSeurat,
        fontSize,
        { textX, min.y + titleTextY },
        isUnlocked ? IM_COL32(252, 243, 5, 255) : colLockedText,
        achievement.Name.c_str(),
        shadowOffset,
        0.4f,
        colTextShadow
    );

    if (isSelected && textX + textSize.x >= max.x)
    {
        // Draw achievement description with marquee.
        DrawTextWithMarqueeShadow
        (
            m_fntSeurat,
            fontSize,
            { textX, min.y + descTextY },
            { textMarqueeX, min.y },
            max,
            isUnlocked ? IM_COL32(255, 255, 255, 255) : colLockedText,
            desc,
            m_rowSelectionTime,
            0.9,
            250.0,
            shadowOffset,
            0.4f,
            colTextShadow
        );
    }
    else
    {
        // Draw achievement description.
        DrawTextWithShadow
        (
            m_fntSeurat,
            fontSize,
            { textX, min.y + descTextY },
            isUnlocked ? IM_COL32(255, 255, 255, 255) : colLockedText,
            desc,
            shadowOffset,
            0.4f,
            colTextShadow
        );
    }

    drawList->PopClipRect();
}

static void DrawContentContainer()
{
    auto drawList = ImGui::GetForegroundDrawList();

    ImVec2 min = { Scale(256), Scale(192) };
    ImVec2 max = { Scale(1026), Scale(601) };

    DrawContainer(min, max, IM_COL32(197, 194, 197, 200), IM_COL32(115, 113, 115, 236));

    auto clipRectMin = drawList->GetClipRectMin();
    auto clipRectMax = drawList->GetClipRectMax();

    auto itemHeight = Scale(94);
    auto yOffset = -m_firstVisibleRowIndex * itemHeight + Scale(2);
    auto rowCount = 0;

    // Draw separators.
    for (int i = 1; i <= 3; i++)
    {
        auto lineMarginLeft = Scale(31);
        auto lineMarginRight = Scale(46);
        auto lineMarginY = Scale(2);

        ImVec2 lineMin = { clipRectMin.x + lineMarginLeft, clipRectMin.y + itemHeight * i + lineMarginY };
        ImVec2 lineMax = { clipRectMax.x - lineMarginRight, clipRectMin.y + itemHeight * i + lineMarginY };

        drawList->AddLine(lineMin, lineMax, IM_COL32(163, 163, 163, 255));
        drawList->AddLine({ lineMin.x, lineMin.y + Scale(1) }, { lineMax.x, lineMax.y + Scale(1) }, IM_COL32(143, 148, 143, 255));
    }

    for (auto achievement : m_achievements)
    {
        if (AchievementData::IsUnlocked(achievement.ID))
            DrawAchievement(rowCount++, yOffset, achievement, true);
    }

    for (auto achievement : m_achievements)
    {
        if (!AchievementData::IsUnlocked(achievement.ID))
            DrawAchievement(rowCount++, yOffset, achievement, false);
    }

    auto inputState = SWA::CInputState::GetInstance();

    bool upIsHeld = inputState->GetPadState().IsDown(SWA::eKeyState_DpadUp) ||
        inputState->GetPadState().LeftStickVertical > 0.5f;

    bool downIsHeld = inputState->GetPadState().IsDown(SWA::eKeyState_DpadDown) ||
        inputState->GetPadState().LeftStickVertical < -0.5f;

    bool scrollUp = !m_upWasHeld && upIsHeld;
    bool scrollDown = !m_downWasHeld && downIsHeld;

    int prevSelectedRowIndex = m_selectedRowIndex;

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
        Game_PlaySound("sys_actstg_pausecursor");
    }

    m_upWasHeld = upIsHeld;
    m_downWasHeld = downIsHeld;

    int visibleRowCount = int(floor((clipRectMax.y - clipRectMin.y) / itemHeight));

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

    // Pop clip rect from DrawContentContainer
    drawList->PopClipRect();

    // Draw scroll bar
    if (rowCount > visibleRowCount)
    {
        float cornerRadius = Scale(25.0f);
        float totalHeight = (clipRectMax.y - clipRectMin.y - cornerRadius) - Scale(3.0f);
        float heightRatio = float(visibleRowCount) / float(rowCount);
        float offsetRatio = float(m_firstVisibleRowIndex) / float(rowCount);
        float offsetX = clipRectMax.x - Scale(31.0f);
        float offsetY = offsetRatio * totalHeight + clipRectMin.y + Scale(4.0f);
        float lineThickness = Scale(1.0f);
        float innerMarginX = Scale(2.0f);
        float outerMarginX = Scale(16.0f);

        // Outline
        drawList->AddRect
        (
            { /* X */ offsetX - lineThickness, /* Y */ clipRectMin.y - lineThickness },
            { /* X */ clipRectMax.x - outerMarginX + lineThickness, /* Y */ max.y - cornerRadius + lineThickness },
            IM_COL32(255, 255, 255, 155),
            Scale(0.5f)
        );

        // Background
        drawList->AddRectFilledMultiColor
        (
            { /* X */ offsetX, /* Y */ clipRectMin.y },
            { /* X */ clipRectMax.x - outerMarginX, /* Y */ max.y - cornerRadius },
            IM_COL32(82, 85, 82, 186),
            IM_COL32(82, 85, 82, 186),
            IM_COL32(74, 73, 74, 185),
            IM_COL32(74, 73, 74, 185)
        );

        // Scroll Bar Outline
        drawList->AddRectFilledMultiColor
        (
            { /* X */ offsetX + innerMarginX, /* Y */ offsetY - lineThickness },
            { /* X */ clipRectMax.x - outerMarginX - innerMarginX, /* Y */ offsetY + lineThickness + totalHeight * heightRatio },
            IM_COL32(185, 185, 185, 255),
            IM_COL32(185, 185, 185, 255),
            IM_COL32(172, 172, 172, 255),
            IM_COL32(172, 172, 172, 255)
        );

        // Scroll Bar
        drawList->AddRectFilled
        (
            { /* X */ offsetX + innerMarginX + lineThickness, /* Y */ offsetY },
            { /* X */ clipRectMax.x - outerMarginX - innerMarginX - lineThickness, /* Y */ offsetY + totalHeight * heightRatio },
            IM_COL32(255, 255, 255, 255)
        );
    }
}

void AchievementMenu::Init()
{
    auto& io = ImGui::GetIO();

    constexpr float FONT_SCALE = 2.0f;

    m_fntSeurat = io.Fonts->AddFontFromFileTTF("FOT-SeuratPro-M.otf", 24.0f * FONT_SCALE);
    m_fntNewRodin = io.Fonts->AddFontFromFileTTF("FOT-NewRodinPro-UB.otf", 20.0f * FONT_SCALE);

    m_achievements = g_xdbfWrapper.GetAchievements((EXDBFLanguage)Config::Language.Value);
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
