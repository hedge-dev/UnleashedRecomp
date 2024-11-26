#include "achievement_overlay.h"
#include "imgui_utils.h"

#include <user/config.h>
#include <user/achievement_data.h>
#include <gpu/video.h>
#include <kernel/memory.h>
#include <locale/locale.h>
#include <app.h>
#include <exports.h>

constexpr double OVERLAY_CONTAINER_MOTION_START = 0.0;
constexpr double OVERLAY_CONTAINER_MOTION_END = 8.0;
constexpr double OVERLAY_CONTAINER_FADE_IN_START = 5.0;
constexpr double OVERLAY_CONTAINER_FADE_OUT_START = 4.0;
constexpr double OVERLAY_ELEMENTS_FADE_START = 10.0;
constexpr double OVERLAY_ELEMENTS_FADE_END = 12.0;
constexpr double OVERLAY_DURATION = 5.0;

static bool g_isClosing = false;

static double g_appearTime = 0.0;

static Achievement g_achievement;
static std::unique_ptr<GuestTexture> g_upAchievementIcon;

static ImFont* g_fntSeurat;

void AchievementOverlay::Init()
{
    auto& io = ImGui::GetIO();

    g_fntSeurat = io.Fonts->AddFontFromFileTTF("FOT-SeuratPro-M.otf", 30.0f);
}

static double ComputeMotion(double frameOffset, double frames)
{
    double t = std::clamp((ImGui::GetTime() - g_appearTime - frameOffset / 60.0) / frames * 60.0, 0.0, 1.0);
    return sqrt(t);
}

// TODO: move this somewhere where it can be re-used.
void DrawContainer(ImVec2 min, ImVec2 max, float cornerRadius = 25.0f)
{
    auto drawList = ImGui::GetForegroundDrawList();

    auto containerMotion = ComputeMotion(OVERLAY_CONTAINER_MOTION_START, OVERLAY_CONTAINER_MOTION_END);
    auto centreX = (min.x + max.x) / 2.0f;
    auto centreY = (min.y + max.y) / 2.0f;

    if (g_isClosing)
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

    ImVec2 v1 = { min.x, min.y + cornerRadius };
    ImVec2 v2 = { min.x + cornerRadius, min.y };
    ImVec2 v3 = { max.x, min.y };
    ImVec2 v4 = { max.x, min.y + cornerRadius };
    ImVec2 v5 = { max.x, max.y - cornerRadius };
    ImVec2 v6 = { max.x - cornerRadius, max.y };
    ImVec2 v7 = { min.x, max.y };
    ImVec2 v8 = { min.x, max.y - cornerRadius };

    ImVec2 top[] = { v1, v2, v3, v4 };
    ImVec2 bottom[] = { v5, v6, v7, v8 };
    ImVec2 border[] = { v1, v2, v3, v4, v5, v6, v7, v8 };

    auto colourMotion = ComputeMotion(g_isClosing ? OVERLAY_CONTAINER_MOTION_START : OVERLAY_CONTAINER_FADE_IN_START,
        g_isClosing ? OVERLAY_CONTAINER_FADE_OUT_START : OVERLAY_CONTAINER_MOTION_END);

    auto colShadow = IM_COL32(0, 0, 0, (int)CubicEase(g_isClosing ? 156 : 0, g_isClosing ? 0 : 156, colourMotion));
    auto colGradientTop = IM_COL32(197, 194, 197, (int)CubicEase(g_isClosing ? 200 : 0, g_isClosing ? 0 : 200, colourMotion));
    auto colGradientBottom = IM_COL32(121, 120, 121, (int)CubicEase(g_isClosing ? 236 : 0, g_isClosing ? 0 : 236, colourMotion)); // TODO: match gradient used by the game (115, 113, 115, 236).

    // TODO: add a drop shadow.

    drawList->AddConvexPolyFilled(top, IM_ARRAYSIZE(top), colGradientTop);
    drawList->AddRectFilledMultiColor({ min.x, min.y + cornerRadius }, { max.x, max.y - cornerRadius }, colGradientTop, colGradientTop, colGradientBottom, colGradientBottom);
    drawList->AddConvexPolyFilled(bottom, IM_ARRAYSIZE(bottom), colGradientBottom);
    drawList->AddPolyline(border, IM_ARRAYSIZE(border), IM_COL32(247, 247, 247, (int)CubicEase(g_isClosing ? 255 : 0, g_isClosing ? 0 : 255, colourMotion)), true, Scale(2.5f));

    for (int i = 0; i < IM_ARRAYSIZE(border); i++)
    {
        border[i].x -= 0.4f;
        border[i].y -= 0.2f;
    }

    auto lineAlpha = (int)CubicEase(g_isClosing ? 230 : 0, g_isClosing ? 0 : 230, colourMotion);
    auto colLineTop = IM_COL32(165, 170, 165, lineAlpha);
    auto colLineBottom = IM_COL32(190, 190, 190, lineAlpha);
    auto lineThickness = Scale(1.0f);

    // Top left corner bottom to top left corner top.
    drawList->AddLine(border[0], border[1], colLineTop, lineThickness * 0.5f);

    // Top left corner bottom to bottom left.
    drawList->AddRectFilledMultiColor({ border[0].x - 0.2f, border[0].y }, { border[6].x + lineThickness - 0.2f, border[6].y }, colLineTop, colLineTop, colLineBottom, colLineBottom);

    // Top left corner top to top right.
    drawList->AddLine(border[1], border[2], colLineTop, lineThickness);

    drawList->PushClipRect(min, max);
}

void AchievementOverlay::Draw()
{
    if (!s_isVisible)
        return;

    if (ImGui::GetTime() - g_appearTime >= OVERLAY_DURATION)
        AchievementOverlay::Close();

    auto drawList = ImGui::GetForegroundDrawList();
    auto& res = ImGui::GetIO().DisplaySize;

    auto strAchievementUnlocked = Localise("Achievements_Unlock").c_str();
    auto strAchievementName = g_achievement.Name.c_str();

    auto fontSize = 30.0f;
    auto headerTextSize = g_fntSeurat->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, strAchievementUnlocked);
    auto bodyTextSize = g_fntSeurat->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, strAchievementName);
    auto longestTextSize = std::max(headerTextSize.x, bodyTextSize.x);

    constexpr auto imageX = 25.0f;
    constexpr auto imageY = 20.0f;
    constexpr auto imageSize = 90.0f;
    constexpr auto textX = 140.0f;

    auto width = textX + longestTextSize + 30.0f;

    ImVec2 min = { (res.x / 2.0f) - (width / 2.0f), 50.0f };
    ImVec2 max = { min.x + width, min.y + 125.0f };

    DrawContainer(min, max);

    auto colourMotion = ComputeMotion(g_isClosing ? OVERLAY_CONTAINER_MOTION_START : OVERLAY_ELEMENTS_FADE_START,
        g_isClosing ? OVERLAY_CONTAINER_FADE_OUT_START : OVERLAY_ELEMENTS_FADE_END);

    auto alpha = (int)CubicEase(g_isClosing ? 255 : 0, g_isClosing ? 0 : 255, colourMotion);

    drawList->AddImage(g_upAchievementIcon.get(), { min.x + imageX, min.y + imageY }, { min.x + imageX + imageSize, min.y + imageY + imageSize }, { 0, 0 }, { 1, 1 }, IM_COL32(255, 255, 255, alpha));

    DrawTextWithShadow(g_fntSeurat, fontSize, { min.x + textX + (longestTextSize - headerTextSize.x) / 2.0f, min.y + 30.0f}, IM_COL32(252, 243, 5, alpha), strAchievementUnlocked, 2.0f, IM_COL32(0, 0, 0, alpha));
    DrawTextWithShadow(g_fntSeurat, fontSize, { min.x + textX + (longestTextSize - bodyTextSize.x) / 2.0f, min.y + 68.0f}, IM_COL32(255, 255, 255, alpha), strAchievementName, 2.0f, IM_COL32(0, 0, 0, alpha));

    // Pop clip rect from DrawContainer
    drawList->PopClipRect();
}

void AchievementOverlay::Open(int id)
{
    s_isVisible = true;
    g_isClosing = false;
    g_appearTime = ImGui::GetTime();

    g_achievement = g_xdbf.GetAchievement((EXDBFLanguage)Config::Language.Value, id);
    g_upAchievementIcon = LoadTexture((uint8_t*)g_achievement.pImageBuffer, g_achievement.ImageBufferSize);

    Game_PlaySound("obj_navi_appear");
}

void AchievementOverlay::Close()
{
    if (!g_isClosing)
    {
        g_appearTime = ImGui::GetTime();
        g_isClosing = true;
    }

    if (ImGui::GetTime() - g_appearTime >= OVERLAY_ELEMENTS_FADE_END)
        s_isVisible = false;
}
