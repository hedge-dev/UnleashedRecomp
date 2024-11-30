#include "achievement_overlay.h"
#include "imgui_utils.h"
#include <gpu/video.h>
#include <kernel/memory.h>
#include <kernel/xdbf.h>
#include <locale/locale.h>
#include <user/config.h>
#include <user/achievement_data.h>
#include <app.h>
#include <exports.h>

constexpr double OVERLAY_CONTAINER_COMMON_MOTION_START = 0;
constexpr double OVERLAY_CONTAINER_COMMON_MOTION_END = 11;
constexpr double OVERLAY_CONTAINER_INTRO_FADE_START = 5;
constexpr double OVERLAY_CONTAINER_INTRO_FADE_END = 9;
constexpr double OVERLAY_CONTAINER_OUTRO_FADE_START = 0;
constexpr double OVERLAY_CONTAINER_OUTRO_FADE_END = 5;

constexpr double OVERLAY_DURATION = 5;

static bool m_isClosing = false;

static double m_appearTime = 0;

static Achievement m_achievement;

static ImFont* m_fntSeurat;

static bool DrawContainer(ImVec2 min, ImVec2 max, float cornerRadius = 25)
{
    auto drawList = ImGui::GetForegroundDrawList();

    // Expand/retract animation.
    auto containerMotion = ComputeMotion(m_appearTime, OVERLAY_CONTAINER_COMMON_MOTION_START, OVERLAY_CONTAINER_COMMON_MOTION_END);

    auto centreX = (min.x + max.x) / 2;
    auto centreY = (min.y + max.y) / 2;

    if (m_isClosing)
    {
        min.x = CubicEase(min.x, centreX, containerMotion);
        max.x = CubicEase(max.x, centreX, containerMotion);
        min.y = CubicEase(min.y, centreY, containerMotion);
        max.y = CubicEase(max.y, centreY, containerMotion);
    }
    else
    {
        min.x = CubicEase(centreX, min.x, containerMotion);
        max.x = CubicEase(centreX, max.x, containerMotion);
        min.y = CubicEase(centreY, min.y, containerMotion);
        max.y = CubicEase(centreY, max.y, containerMotion);
    }

    auto vertices = GetPauseContainerVertices(min, max, cornerRadius);

    // Transparency fade animation.
    auto colourMotion = m_isClosing
        ? ComputeMotion(m_appearTime, OVERLAY_CONTAINER_OUTRO_FADE_START, OVERLAY_CONTAINER_OUTRO_FADE_END)
        : ComputeMotion(m_appearTime, OVERLAY_CONTAINER_INTRO_FADE_START, OVERLAY_CONTAINER_INTRO_FADE_END);

    auto alpha = m_isClosing
        ? CubicEase(1, 0, colourMotion)
        : CubicEase(0, 1, colourMotion);

    auto colShadow = IM_COL32(0, 0, 0, 156 * alpha);
    auto colGradientTop = IM_COL32(197, 194, 197, 200 * alpha);
    auto colGradientBottom = IM_COL32(115, 113, 115, 236 * alpha);

    // TODO: add a drop shadow.

    // Draw vertices with gradient.
    SetGradient(min, max, colGradientTop, colGradientBottom);
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

    drawList->PushClipRect(min, max);

    return containerMotion >= 1.0f;
}

void AchievementOverlay::Init()
{
    auto& io = ImGui::GetIO();

    constexpr float FONT_SCALE = 2.0f;

    m_fntSeurat = io.Fonts->AddFontFromFileTTF("FOT-SeuratPro-M.otf", 24.0f * FONT_SCALE);
}

void AchievementOverlay::Draw()
{
    if (!s_isVisible)
        return;
    
    if (ImGui::GetTime() - m_appearTime >= OVERLAY_DURATION)
        AchievementOverlay::Close();

    auto drawList = ImGui::GetForegroundDrawList();
    auto& res = ImGui::GetIO().DisplaySize;

    auto strAchievementUnlocked = Localise("Achievements_Unlock").c_str();
    auto strAchievementName = m_achievement.Name.c_str();

    // Calculate text sizes.
    auto fontSize = Scale(24);
    auto headerSize = m_fntSeurat->CalcTextSizeA(fontSize, FLT_MAX, 0, strAchievementUnlocked);
    auto bodySize = m_fntSeurat->CalcTextSizeA(fontSize, FLT_MAX, 0, strAchievementName);
    auto maxSize = std::max(headerSize.x, bodySize.x);

    // Calculate image margins.
    auto imageMarginX = Scale(20);
    auto imageMarginY = Scale(20);
    auto imageSize = Scale(60);

    // Calculate text margins.
    auto textMarginX = imageMarginX * 2 + imageSize;
    auto textMarginY = imageMarginY + Scale(2);

    auto containerWidth = imageMarginX + textMarginX + maxSize;

    ImVec2 min = { (res.x / 2) - (containerWidth / 2), Scale(50) };
    ImVec2 max = { min.x + containerWidth, min.y + Scale(100) };

    if (DrawContainer(min, max))
    {
        // Draw achievement icon.
        drawList->AddImage
        (
            g_xdbfTextureCache[m_achievement.ID],                                                   // user_texture_id
            { /* X */ min.x + imageMarginX, /* Y */ min.y + imageMarginY },                         // p_min
            { /* X */ min.x + imageMarginX + imageSize, /* Y */ min.y + imageMarginY + imageSize }, // p_max
            { 0, 0 },                                                                               // uv_min
            { 1, 1 },                                                                               // uv_max
            IM_COL32(255, 255, 255, 255)                                                            // col
        );

        // Draw header text.
        DrawTextWithShadow
        (
            m_fntSeurat,                                                                                 // font
            fontSize,                                                                                    // fontSize
            { /* X */ min.x + textMarginX + (maxSize - headerSize.x) / 2, /* Y */ min.y + textMarginY }, // pos
            IM_COL32(252, 243, 5, 255),                                                                  // colour
            strAchievementUnlocked,                                                                      // text
            2,                                                                                           // offset
            0.4f,                                                                                        // radius
            IM_COL32(0, 0, 0, 255)                                                                       // shadowColour
        );

        // Draw achievement name.
        DrawTextWithShadow
        (
            m_fntSeurat,                                                                                                       // font
            fontSize,                                                                                                          // fontSize
            { /* X */ min.x + textMarginX + (maxSize - bodySize.x) / 2, /* Y */ min.y + textMarginY + bodySize.y + Scale(6) }, // pos
            IM_COL32(255, 255, 255, 255),                                                                                      // colour
            strAchievementName,                                                                                                // text
            2,                                                                                                                 // offset
            0.4f,                                                                                                              // radius
            IM_COL32(0, 0, 0, 255)                                                                                             // shadowColour
        );

        // Pop clip rect from DrawContainer.
        drawList->PopClipRect();
    }
}

void AchievementOverlay::Open(int id)
{
    s_isVisible = true;
    m_isClosing = false;
    m_appearTime = ImGui::GetTime();
    m_achievement = g_xdbfWrapper.GetAchievement((EXDBFLanguage)Config::Language.Value, id);

    Game_PlaySound("obj_navi_appear");
}

void AchievementOverlay::Close()
{
    if (!m_isClosing)
    {
        m_appearTime = ImGui::GetTime();
        m_isClosing = true;
    }

    if (ImGui::GetTime() - m_appearTime >= OVERLAY_CONTAINER_COMMON_MOTION_END)
        s_isVisible = false;
}
