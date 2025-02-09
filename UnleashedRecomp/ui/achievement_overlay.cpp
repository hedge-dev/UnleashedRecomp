#include "achievement_overlay.h"
#include <gpu/imgui/imgui_snapshot.h>
#include <gpu/video.h>
#include <kernel/memory.h>
#include <kernel/xdbf.h>
#include <locale/locale.h>
#include <ui/imgui_utils.h>
#include <user/achievement_data.h>
#include <user/config.h>
#include <app.h>
#include <exports.h>
#include <decompressor.h>

constexpr double OVERLAY_CONTAINER_COMMON_MOTION_START = 0;
constexpr double OVERLAY_CONTAINER_COMMON_MOTION_END = 11;
constexpr double OVERLAY_CONTAINER_INTRO_FADE_START = 5;
constexpr double OVERLAY_CONTAINER_INTRO_FADE_END = 9;
constexpr double OVERLAY_CONTAINER_OUTRO_FADE_START = 0;
constexpr double OVERLAY_CONTAINER_OUTRO_FADE_END = 4;

constexpr double OVERLAY_DURATION = 3;

static bool g_isClosing = false;

static double g_appearTime = 0;

static Achievement g_achievement;

static ImFont* g_fntSeurat;

static bool DrawContainer(ImVec2 min, ImVec2 max, float cornerRadius = 25)
{
    auto drawList = ImGui::GetBackgroundDrawList();

    // Expand/retract animation.
    auto containerMotion = ComputeMotion(g_appearTime, OVERLAY_CONTAINER_COMMON_MOTION_START, OVERLAY_CONTAINER_COMMON_MOTION_END);

    auto centreX = (min.x + max.x) / 2;
    auto centreY = (min.y + max.y) / 2;

    if (g_isClosing)
    {
        min.x = Hermite(min.x, centreX, containerMotion);
        max.x = Hermite(max.x, centreX, containerMotion);
        min.y = Hermite(min.y, centreY, containerMotion);
        max.y = Hermite(max.y, centreY, containerMotion);
    }
    else
    {
        min.x = Hermite(centreX, min.x, containerMotion);
        max.x = Hermite(centreX, max.x, containerMotion);
        min.y = Hermite(centreY, min.y, containerMotion);
        max.y = Hermite(centreY, max.y, containerMotion);
    }

    // Transparency fade animation.
    auto colourMotion = g_isClosing
        ? ComputeMotion(g_appearTime, OVERLAY_CONTAINER_OUTRO_FADE_START, OVERLAY_CONTAINER_OUTRO_FADE_END)
        : ComputeMotion(g_appearTime, OVERLAY_CONTAINER_INTRO_FADE_START, OVERLAY_CONTAINER_INTRO_FADE_END);

    auto alpha = g_isClosing
        ? Hermite(1, 0, colourMotion)
        : Hermite(0, 1, colourMotion);

    DrawPauseContainer(min, max, alpha);

    if (containerMotion >= 1.0f)
    {
        drawList->PushClipRect(min, max);
        return true;
    }

    return false;
}

void AchievementOverlay::Init()
{
    auto& io = ImGui::GetIO();

    g_fntSeurat = ImFontAtlasSnapshot::GetFont("FOT-SeuratPro-M.otf");
}

void AchievementOverlay::Draw()
{
    if (!s_isVisible)
        return;
    
    if (ImGui::GetTime() - g_appearTime >= OVERLAY_DURATION)
        AchievementOverlay::Close();

    auto drawList = ImGui::GetBackgroundDrawList();
    auto& res = ImGui::GetIO().DisplaySize;

    auto strAchievementUnlocked = Localise("Achievements_Unlock").c_str();
    auto strAchievementName = g_achievement.Name.c_str();

    // Calculate text sizes.
    auto fontSize = Scale(24);
    auto headerSize = g_fntSeurat->CalcTextSizeA(fontSize, FLT_MAX, 0, strAchievementUnlocked);
    auto bodySize = g_fntSeurat->CalcTextSizeA(fontSize, FLT_MAX, 0, strAchievementName);
    auto maxSize = std::max(headerSize.x, bodySize.x) + Scale(5);

    // Calculate image margins.
    auto imageMarginX = Scale(25);
    auto imageMarginY = Scale(22.5f);
    auto imageSize = Scale(60);

    // Calculate text margins.
    auto textMarginX = imageMarginX * 2 + imageSize - Scale(5);
    auto textMarginY = imageMarginY + Scale(2);

    auto containerWidth = imageMarginX + textMarginX + maxSize;

    ImVec2 min = { (res.x / 2) - (containerWidth / 2), Scale(55) };
    ImVec2 max = { min.x + containerWidth, min.y + Scale(105) };

    if (DrawContainer(min, max))
    {
        if (!g_isClosing)
        {
            // Draw achievement icon.
            drawList->AddImage
            (
                g_xdbfTextureCache[g_achievement.ID],                                                   // user_texture_id
                { /* X */ min.x + imageMarginX, /* Y */ min.y + imageMarginY },                         // p_min
                { /* X */ min.x + imageMarginX + imageSize, /* Y */ min.y + imageMarginY + imageSize }, // p_max
                { 0, 0 },                                                                               // uv_min
                { 1, 1 },                                                                               // uv_max
                IM_COL32(255, 255, 255, 255)                                                            // col
            );

            // Draw header text.
            DrawTextWithShadow
            (
                g_fntSeurat,                                                                                 // font
                fontSize,                                                                                    // fontSize
                { /* X */ min.x + textMarginX + (maxSize - headerSize.x) / 2, /* Y */ min.y + textMarginY }, // pos
                IM_COL32(252, 243, 5, 255),                                                                  // colour
                strAchievementUnlocked,                                                                      // text
                2,                                                                                           // offset
                1.0f,                                                                                        // radius
                IM_COL32(0, 0, 0, 255)                                                                       // shadowColour
            );

            // Draw achievement name.
            DrawTextWithShadow
            (
                g_fntSeurat,                                                                                                       // font
                fontSize,                                                                                                          // fontSize
                { /* X */ min.x + textMarginX + (maxSize - bodySize.x) / 2, /* Y */ min.y + textMarginY + bodySize.y + Scale(6) }, // pos
                IM_COL32(255, 255, 255, 255),                                                                                      // colour
                strAchievementName,                                                                                                // text
                2,                                                                                                                 // offset
                1.0f,                                                                                                              // radius
                IM_COL32(0, 0, 0, 255)                                                                                             // shadowColour
            );
        }
        else
        {
            s_isVisible = false;
        }

        // Pop clip rect from DrawContainer.
        drawList->PopClipRect();
    }
}

void AchievementOverlay::Open(int id)
{
    if (s_isVisible && !g_isClosing)
    {
        s_queue.emplace(id);
        return;
    }

    s_isVisible = true;
    g_isClosing = false;
    g_appearTime = ImGui::GetTime();
    g_achievement = g_xdbfWrapper.GetAchievement((EXDBFLanguage)Config::Language.Value, id);

    if (Config::Language == ELanguage::English)
        g_achievement.Name = xdbf::FixInvalidSequences(g_achievement.Name);

    Game_PlaySound("obj_navi_appear");
}

void AchievementOverlay::Close()
{
    if (!g_isClosing)
    {
        g_appearTime = ImGui::GetTime();
        g_isClosing = true;
    }

    if (s_queue.size())
    {
        s_isVisible = false;
        AchievementOverlay::Open(s_queue.front());
        s_queue.pop();
    }
}
