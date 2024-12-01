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

constexpr double HEADER_CONTAINER_INTRO_MOTION_START = 0;
constexpr double HEADER_CONTAINER_INTRO_MOTION_END = 15;
constexpr double HEADER_CONTAINER_OUTRO_MOTION_START = 0;
constexpr double HEADER_CONTAINER_OUTRO_MOTION_END = 40;
constexpr double HEADER_CONTAINER_INTRO_FADE_START = 5;
constexpr double HEADER_CONTAINER_INTRO_FADE_END = 14;
constexpr double HEADER_CONTAINER_OUTRO_FADE_START = 0;
constexpr double HEADER_CONTAINER_OUTRO_FADE_END = 7;

constexpr double CONTENT_CONTAINER_COMMON_MOTION_START = 11;
constexpr double CONTENT_CONTAINER_COMMON_MOTION_END = 12;

constexpr double COUNTER_INTRO_FADE_START = 15;
constexpr double COUNTER_INTRO_FADE_END = 16;
constexpr double COUNTER_SPRITE_FRAME_START = 0;
constexpr double COUNTER_SPRITE_FRAME_END = 30;

constexpr double SELECTION_CONTAINER_BREATHE = 30;

static bool g_isClosing = false;

static double g_appearTime = 0;

static std::vector<std::tuple<Achievement, time_t>> g_achievements;

static ImFont* g_fntSeurat;
static ImFont* g_fntNewRodinDB;
static ImFont* g_fntNewRodinUB;

static std::unique_ptr<GuestTexture> g_upTrophyIcon;

static int g_firstVisibleRowIndex;
static int g_selectedRowIndex;
static double g_rowSelectionTime;

static bool g_upWasHeld;
static bool g_downWasHeld;
static bool g_leftWasHeld;
static bool g_rightWasHeld;
static bool g_upRSWasHeld;
static bool g_downRSWasHeld;

static void ResetSelection()
{
    g_firstVisibleRowIndex = 0;
    g_selectedRowIndex = 0;
    g_rowSelectionTime = ImGui::GetTime();
    g_upWasHeld = false;
    g_downWasHeld = false;
}

static void DrawContainer(ImVec2 min, ImVec2 max, ImU32 gradientTop, ImU32 gradientBottom, float alpha = 1, float cornerRadius = 25)
{
    auto drawList = ImGui::GetForegroundDrawList();
    auto vertices = GetPauseContainerVertices(min, max, cornerRadius);

    // TODO: add a drop shadow.

    SetGradient(min, max, gradientTop, gradientBottom);
    drawList->AddConvexPolyFilled(vertices.data(), vertices.size(), IM_COL32(255, 255, 255, 255 * alpha));
    ResetGradient();

    drawList->AddPolyline(vertices.data(), vertices.size(), IM_COL32(247, 247, 247, 255 * alpha), true, Scale(2.5f));

    for (int i = 0; i < vertices.size(); i++)
    {
        vertices[i].x -= Scale(0.4f);
        vertices[i].y -= Scale(0.2f);
    }

    auto colLineTop = IM_COL32(165, 170, 165, 230 * alpha);
    auto colLineBottom = IM_COL32(190, 190, 190, 230 * alpha);
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

    static auto breatheStart = ImGui::GetTime();
    auto alpha = Lerp(1.0f, 0.75f, (sin((ImGui::GetTime() - breatheStart) * (2.0f * M_PI / (55.0f / 60.0f))) + 1.0f) / 2.0f);

    SetGradient(min, max, IM_COL32(255, 246, 0, 129), IM_COL32(255, 194, 0, 118 * alpha));
    drawList->AddConvexPolyFilled(vertices.data(), vertices.size(), IM_COL32(255, 255, 255, 255 * alpha));
    ResetGradient();
}

static void DrawHeaderContainer(const char* text)
{
    auto drawList = ImGui::GetForegroundDrawList();
    auto fontSize = Scale(26);
    auto textSize = g_fntNewRodinUB->CalcTextSizeA(fontSize, FLT_MAX, 0, text);
    auto cornerRadius = 23;
    auto textMarginX = Scale(16) + (Scale(cornerRadius) / 2);

    auto containerMotion = g_isClosing
        ? ComputeMotion(g_appearTime, HEADER_CONTAINER_OUTRO_MOTION_START, HEADER_CONTAINER_OUTRO_MOTION_END)
        : ComputeMotion(g_appearTime, HEADER_CONTAINER_INTRO_MOTION_START, HEADER_CONTAINER_INTRO_MOTION_END);

    auto colourMotion = g_isClosing
        ? ComputeMotion(g_appearTime, HEADER_CONTAINER_OUTRO_FADE_START, HEADER_CONTAINER_OUTRO_FADE_END)
        : ComputeMotion(g_appearTime, HEADER_CONTAINER_INTRO_FADE_START, HEADER_CONTAINER_INTRO_FADE_END);

    // Slide animation.
    auto containerMarginX = g_isClosing
        ? Hermite(256, 156, containerMotion)
        : Hermite(156, 256, containerMotion);

    // Transparency fade animation.
    auto alpha = g_isClosing
        ? Lerp(1, 0, colourMotion)
        : Lerp(0, 1, colourMotion);

    ImVec2 min = { Scale(containerMarginX), Scale(138) };
    ImVec2 max = { min.x + textMarginX * 2 + textSize.x, Scale(185) };

    DrawContainer(min, max, IM_COL32(140, 142, 140, 201), IM_COL32(66, 65, 66, 234), alpha, cornerRadius);
    drawList->PopClipRect();

    // TODO: skew this text and apply bevel.
    DrawTextWithOutline<int>
    (
        g_fntNewRodinUB,
        fontSize,
        { /* X */ min.x + textMarginX, /* Y */ min.y + ((max.y - min.y) - textSize.y) / 2 },
        IM_COL32(255, 255, 255, 255 * alpha),
        text,
        3,
        IM_COL32(0, 0, 0, 255 * alpha)
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
    auto isSelected = rowIndex == g_selectedRowIndex;

    if (isSelected)
        DrawSelectionContainer(min, max);

    auto desc = isUnlocked ? achievement.UnlockedDesc.c_str() : achievement.LockedDesc.c_str();
    auto fontSize = Scale(24);
    auto textSize = g_fntSeurat->CalcTextSizeA(fontSize, FLT_MAX, 0, desc);
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
        g_fntSeurat,
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
            g_fntSeurat,
            fontSize,
            { textX, min.y + descTextY },
            { textMarqueeX, min.y },
            max,
            isUnlocked ? IM_COL32(255, 255, 255, 255) : colLockedText,
            desc,
            g_rowSelectionTime,
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
            g_fntSeurat,
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

    if (!isUnlocked)
        return;

    auto timestamp = AchievementData::GetTimestamp(achievement.ID);

    if (!timestamp)
        return;

    char buffer[32];
    struct tm time;
    localtime_s(&time, &timestamp);
    strftime(buffer, sizeof(buffer), "%Y/%m/%d %H:%M", &time);

    fontSize = Scale(14);
    textSize = g_fntNewRodinDB->CalcTextSizeA(fontSize, FLT_MAX, 0, buffer);

    auto containerMarginX = Scale(10);
    auto textMarginX = Scale(3);

    ImVec2 timestampMin = { max.x - containerMarginX - textSize.x - (textMarginX * 2), min.y + titleTextY };
    ImVec2 timestampMax = { max.x - containerMarginX, min.y + Scale(46) };

    drawList->PushClipRect(min, max, true);

    auto bevelOffset = Scale(6);

    // Left
    drawList->AddRectFilledMultiColor
    (
        timestampMin,
        { timestampMin.x + bevelOffset, timestampMax.y },
        IM_COL32(255, 255, 255, 255),
        IM_COL32(149, 149, 149, 40),
        IM_COL32(149, 149, 149, 40),
        IM_COL32(255, 255, 255, 255)
    );

    // Right
    drawList->AddRectFilledMultiColor
    (
        { timestampMax.x - bevelOffset, timestampMin.y },
        { timestampMax.x, timestampMax.y },
        IM_COL32(149, 149, 149, 40),
        IM_COL32(255, 255, 255, 255),
        IM_COL32(255, 255, 255, 255),
        IM_COL32(149, 149, 149, 40)
    );

    // Centre
    drawList->AddRectFilled
    (
        { timestampMin.x, timestampMin.y + bevelOffset },
        { timestampMax.x, timestampMax.y - bevelOffset },
        IM_COL32(38, 38, 38, 172)
    );

    // Top
    drawList->AddRectFilledMultiColor
    (
        timestampMin,
        { timestampMax.x, timestampMin.y + bevelOffset },
        IM_COL32(16, 16, 16, 192),
        IM_COL32(16, 16, 16, 192),
        IM_COL32(38, 38, 38, 172),
        IM_COL32(38, 38, 38, 172)
    );

    // Bottom
    drawList->AddRectFilledMultiColor
    (
        { timestampMin.x, timestampMax.y - bevelOffset },
        { timestampMax.x, timestampMax.y },
        IM_COL32(38, 40, 38, 169),
        IM_COL32(38, 40, 38, 169),
        IM_COL32(16, 16, 16, 192),
        IM_COL32(16, 16, 16, 192)
    );

    DrawTextWithOutline<int>
    (
        g_fntNewRodinDB,
        fontSize,
        { /* X */ timestampMin.x + textMarginX, /* Y */ timestampMin.y + ((timestampMax.y - timestampMin.y) - textSize.y) / 2 },
        IM_COL32(255, 255, 255, 255),
        buffer,
        2,
        IM_COL32(8, 8, 8, 255)
    );

    drawList->PopClipRect();
}

static void DrawAchievementTotal(ImVec2 min, ImVec2 max)
{
    auto drawList = ImGui::GetForegroundDrawList();

    // Transparency fade animation.
    auto alpha = Cubic(0, 1, ComputeMotion(g_appearTime, COUNTER_INTRO_FADE_START, COUNTER_INTRO_FADE_END));

    auto imageMarginX = Scale(5);
    auto imageMarginY = Scale(5);
    auto imageSize = Scale(45);

    ImVec2 imageMin = { max.x - imageSize - imageMarginX, min.y - imageSize - imageMarginY };
    ImVec2 imageMax = { imageMin.x + imageSize, imageMin.y + imageSize };

    auto frm = int32_t(floor(ImGui::GetTime() * 30.0f)) % 30;
    auto w = 256.0f / 7680.0f;
    auto uv0 = ImVec2(frm * w, 0);
    auto uv1 = ImVec2((frm + 1) * w, 1);

    drawList->AddImage(g_upTrophyIcon.get(), imageMin, imageMax, uv0, uv1, IM_COL32(255, 255, 255, 255 * alpha));

    auto str = std::format("{} / {}", AchievementData::GetTotalRecords(), ACH_RECORDS);
    auto fontSize = Scale(20);
    auto textSize = g_fntNewRodinDB->CalcTextSizeA(fontSize, FLT_MAX, 0, str.c_str());

    DrawTextWithOutline<int>
    (
        g_fntNewRodinDB,
        fontSize,
        { /* X */ imageMin.x - textSize.x - Scale(6), /* Y */ imageMin.y + ((imageMax.y - imageMin.y) - textSize.y) / 2 },
        IM_COL32(255, 255, 255, 255 * alpha),
        str.c_str(),
        2,
        IM_COL32(0, 0, 0, 255 * alpha)
    );
}

static void DrawContentContainer()
{
    auto drawList = ImGui::GetForegroundDrawList();

    // Expand/retract animation.
    auto motion = g_isClosing
        ? ComputeMotion(g_appearTime, 0, CONTENT_CONTAINER_COMMON_MOTION_START)
        : ComputeMotion(g_appearTime, CONTENT_CONTAINER_COMMON_MOTION_START, CONTENT_CONTAINER_COMMON_MOTION_END);

    auto minX = g_isClosing
        ? Hermite(256, 306, motion)
        : Hermite(306, 256, motion);

    auto minY = g_isClosing
        ? Hermite(192, 209, motion)
        : Hermite(209, 192, motion);

    auto maxX = g_isClosing
        ? Hermite(1026, 973, motion)
        : Hermite(973, 1026, motion);

    auto maxY = g_isClosing
        ? Hermite(601, 569, motion)
        : Hermite(569, 601, motion);

    ImVec2 min = { Scale(minX), Scale(minY) };
    ImVec2 max = { Scale(maxX), Scale(maxY) };

    // Transparency fade animation.
    auto alpha = g_isClosing
        ? Hermite(1, 0, motion)
        : Hermite(0, 1, motion);

    DrawContainer(min, max, IM_COL32(197, 194, 197, 200), IM_COL32(115, 113, 115, 236), alpha);

    if (motion < 1.0f)
    {
        return;
    }
    else if (g_isClosing)
    {
        AchievementMenu::s_isVisible = false;
        return;
    }

    auto clipRectMin = drawList->GetClipRectMin();
    auto clipRectMax = drawList->GetClipRectMax();

    auto itemHeight = Scale(94);
    auto yOffset = -g_firstVisibleRowIndex * itemHeight + Scale(2);
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

    for (auto& tpl : g_achievements)
    {
        auto achievement = std::get<0>(tpl);

        if (AchievementData::IsUnlocked(achievement.ID))
            DrawAchievement(rowCount++, yOffset, achievement, true);
    }

    for (auto& tpl : g_achievements)
    {
        auto achievement = std::get<0>(tpl);

        if (!AchievementData::IsUnlocked(achievement.ID))
            DrawAchievement(rowCount++, yOffset, achievement, false);
    }

    auto inputState = SWA::CInputState::GetInstance();

    bool upIsHeld = inputState->GetPadState().IsDown(SWA::eKeyState_DpadUp) ||
        inputState->GetPadState().LeftStickVertical > 0.5f;

    bool downIsHeld = inputState->GetPadState().IsDown(SWA::eKeyState_DpadDown) ||
        inputState->GetPadState().LeftStickVertical < -0.5f;

    bool leftIsHeld = inputState->GetPadState().IsDown(SWA::eKeyState_DpadLeft) ||
        inputState->GetPadState().LeftStickHorizontal < -0.5f;

    bool rightIsHeld = inputState->GetPadState().IsDown(SWA::eKeyState_DpadRight) ||
        inputState->GetPadState().LeftStickHorizontal > 0.5f;

    bool upRSIsHeld = inputState->GetPadState().RightStickVertical > 0.5f;
    bool downRSIsHeld = inputState->GetPadState().RightStickVertical < -0.5f;

    bool isReachedTop = g_selectedRowIndex == 0;
    bool isReachedBottom = g_selectedRowIndex == rowCount - 1;

    bool scrollUp = !g_upWasHeld && upIsHeld;
    bool scrollDown = !g_downWasHeld && downIsHeld;
    bool scrollPageUp = !g_leftWasHeld && leftIsHeld && !isReachedTop;
    bool scrollPageDown = !g_rightWasHeld && rightIsHeld && !isReachedBottom;
    bool jumpToTop = !g_upRSWasHeld && upRSIsHeld && !isReachedTop;
    bool jumpToBottom = !g_downRSWasHeld && downRSIsHeld && !isReachedBottom;

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
    else if (scrollPageUp)
    {
        g_selectedRowIndex -= 3;
        if (g_selectedRowIndex < 0)
            g_selectedRowIndex = 0;
    }
    else if (scrollPageDown)
    {
        g_selectedRowIndex += 3;
        if (g_selectedRowIndex >= rowCount)
            g_selectedRowIndex = rowCount - 1;
    }
    else if (jumpToTop)
    {
        g_selectedRowIndex = 0;
    }
    else if (jumpToBottom)
    {
        g_selectedRowIndex = rowCount - 1;
    }

    // lol
    if (scrollUp || scrollDown || scrollPageUp || scrollPageDown || jumpToTop || jumpToBottom)
    {
        g_rowSelectionTime = ImGui::GetTime();
        Game_PlaySound("sys_actstg_pausecursor");
    }

    g_upWasHeld = upIsHeld;
    g_downWasHeld = downIsHeld;
    g_leftWasHeld = leftIsHeld;
    g_rightWasHeld = rightIsHeld;
    g_upRSWasHeld = upRSIsHeld;
    g_downRSWasHeld = downRSIsHeld;

    int visibleRowCount = int(floor((clipRectMax.y - clipRectMin.y) / itemHeight));

    if (g_firstVisibleRowIndex > g_selectedRowIndex)
        g_firstVisibleRowIndex = g_selectedRowIndex;

    if (g_firstVisibleRowIndex + visibleRowCount - 1 < g_selectedRowIndex)
        g_firstVisibleRowIndex = std::max(0, g_selectedRowIndex - visibleRowCount + 1);

    // Pop clip rect from DrawContentContainer
    drawList->PopClipRect();

    DrawAchievementTotal(min, max);

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

    g_fntSeurat = io.Fonts->AddFontFromFileTTF("FOT-SeuratPro-M.otf", 24.0f * FONT_SCALE);
    g_fntNewRodinDB = io.Fonts->AddFontFromFileTTF("FOT-NewRodinPro-DB.otf", 20.0f * FONT_SCALE);
    g_fntNewRodinUB = io.Fonts->AddFontFromFileTTF("FOT-NewRodinPro-UB.otf", 20.0f * FONT_SCALE);

    size_t bufferSize = 0;
    auto buffer = ReadAllBytes("achievements_trophy.png", bufferSize);

    if (!bufferSize)
        return;

    g_upTrophyIcon = LoadTexture(buffer.get(), bufferSize);
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
    g_isClosing = false;
    g_appearTime = ImGui::GetTime();

    g_achievements.clear();

    for (auto& achievement : g_xdbfWrapper.GetAchievements((EXDBFLanguage)Config::Language.Value))
        g_achievements.push_back(std::make_tuple(achievement, AchievementData::GetTimestamp(achievement.ID)));

    std::sort(g_achievements.begin(), g_achievements.end(), [](const auto& a, const auto& b)
    {
        return std::get<1>(a) > std::get<1>(b);
    });

    ResetSelection();
    Game_PlaySound("sys_actstg_pausewinopen");
}

void AchievementMenu::Close()
{
    if (!g_isClosing)
    {
        g_appearTime = ImGui::GetTime();
        g_isClosing = true;
    }

    Game_PlaySound("sys_actstg_pausewinclose");
    Game_PlaySound("sys_actstg_pausecansel");
}
