#include "button_guide.h"
#include "imgui_utils.h"
#include <gpu/imgui_snapshot.h>
#include <gpu/video.h>
#include <decompressor.h>
#include <res/images/common/left_mouse_button.dds.h>
#include <res/images/common/mat_comon_x360_001.dds.h>
#include <res/images/common/start_back.dds.h>

constexpr float DEFAULT_SIDE_MARGINS = 379;

ImFont* g_fntNewRodin;
ImFont* g_fntNewRodinLQ;

std::unique_ptr<GuestTexture> g_upIcons;
std::unique_ptr<GuestTexture> g_upLMBIcon;
std::unique_ptr<GuestTexture> g_upStartBackIcons;

float g_sideMargins = DEFAULT_SIDE_MARGINS;

std::vector<Button> g_buttons;

std::unordered_map<EButtonIcon, float> g_iconWidths =
{
    { EButtonIcon::A, 40 },
    { EButtonIcon::B, 40 },
    { EButtonIcon::X, 40 },
    { EButtonIcon::Y, 40 },
    { EButtonIcon::LB, 70 },
    { EButtonIcon::RB, 70 },
    { EButtonIcon::LBRB, 70 },
    { EButtonIcon::LT, 42 },
    { EButtonIcon::RT, 42 },
    { EButtonIcon::LTRT, 42 },
    { EButtonIcon::Start, 40 },
    { EButtonIcon::Back, 40 },
    { EButtonIcon::LMB, 40 }
};

std::unordered_map<EButtonIcon, float> g_iconHeights =
{
    { EButtonIcon::A, 40 },
    { EButtonIcon::B, 40 },
    { EButtonIcon::X, 40 },
    { EButtonIcon::Y, 40 },
    { EButtonIcon::LB, 40 },
    { EButtonIcon::RB, 40 },
    { EButtonIcon::LBRB, 40 },
    { EButtonIcon::LT, 42 },
    { EButtonIcon::RT, 42 },
    { EButtonIcon::LTRT, 42 },
    { EButtonIcon::Start, 40 },
    { EButtonIcon::Back, 40 },
    { EButtonIcon::LMB, 40 }
};

std::tuple<std::tuple<ImVec2, ImVec2>, GuestTexture*> GetButtonIcon(EButtonIcon icon)
{
    std::tuple<ImVec2, ImVec2> btn;
    GuestTexture* texture;

    switch (icon)
    {
        case EButtonIcon::A:
            btn = PIXELS_TO_UV_COORDS(512, 512, 0, 0, 40, 40);
            texture = g_upIcons.get();
            break;

        case EButtonIcon::B:
            btn = PIXELS_TO_UV_COORDS(512, 512, 40, 0, 40, 40);
            texture = g_upIcons.get();
            break;

        case EButtonIcon::X:
            btn = PIXELS_TO_UV_COORDS(512, 512, 80, 0, 40, 40);
            texture = g_upIcons.get();
            break;

        case EButtonIcon::Y:
            btn = PIXELS_TO_UV_COORDS(512, 512, 120, 0, 40, 40);
            texture = g_upIcons.get();
            break;

        case EButtonIcon::LB:
            btn = PIXELS_TO_UV_COORDS(512, 512, 166, 0, 70, 40);
            texture = g_upIcons.get();
            break;

        case EButtonIcon::RB:
            btn = PIXELS_TO_UV_COORDS(512, 512, 246, 0, 70, 40);
            texture = g_upIcons.get();
            break;

        case EButtonIcon::LT:
            btn = PIXELS_TO_UV_COORDS(512, 512, 319, 0, 42, 42);
            texture = g_upIcons.get();
            break;

        case EButtonIcon::RT:
            btn = PIXELS_TO_UV_COORDS(512, 512, 359, 0, 42, 42);
            texture = g_upIcons.get();
            break;

        case EButtonIcon::Start:
            btn = PIXELS_TO_UV_COORDS(256, 256, 0, 0, 128, 128);
            texture = g_upStartBackIcons.get();
            break;

        case EButtonIcon::Back:
            btn = PIXELS_TO_UV_COORDS(256, 256, 0, 128, 128, 128);
            texture = g_upStartBackIcons.get();
            break;

        case EButtonIcon::LMB:
            btn = PIXELS_TO_UV_COORDS(128, 128, 0, 0, 128, 128);
            texture = g_upLMBIcon.get();
            break;
    }

    return std::make_tuple(btn, texture);
}

ImFont* GetFont(EFontQuality fontQuality)
{
    return fontQuality == EFontQuality::Low
        ? g_fntNewRodinLQ
        : g_fntNewRodin;
}

static void DrawGuide(float* offset, ImVec2 regionMin, ImVec2 regionMax, EButtonIcon icon,
    EButtonAlignment alignment, ImVec2 iconMin, ImVec2 iconMax, EFontQuality fontQuality,
    ImVec2 textSize, float fontSize, const char* text)
{
    auto drawList = ImGui::GetForegroundDrawList();
    auto _icon = icon;
    auto iconWidth = Scale(g_iconWidths[icon]);
    auto dualIconMarginX = Scale(25);

    if (icon == EButtonIcon::LBRB)
    {
        _icon = EButtonIcon::LB;
    }
    else if (icon == EButtonIcon::LTRT)
    {
        _icon = EButtonIcon::LT;
    }
    else
    {
        dualIconMarginX = 0;
    }

    if (icon == EButtonIcon::LBRB || icon == EButtonIcon::LTRT)
    {
        iconMin = alignment == EButtonAlignment::Left
            ? ImVec2(/* X */ regionMin.x + *offset - dualIconMarginX * 3, /* Y */ iconMin.y)
            : ImVec2(/* X */ regionMax.x - *offset - textSize.x - iconWidth, /* Y */ iconMin.y);

        iconMax = alignment == EButtonAlignment::Left
            ? ImVec2(iconMin.x + iconWidth, iconMax.y)
            : ImVec2(iconMin.x, iconMax.y);
    }

    auto btnIcon = GetButtonIcon(_icon);
    drawList->AddImage(std::get<1>(btnIcon), iconMin, iconMax, GET_UV_COORDS(std::get<0>(btnIcon)));

    auto textMarginX = alignment == EButtonAlignment::Left
        ? regionMin.x + *offset + dualIconMarginX
        : regionMax.x - *offset - dualIconMarginX * 2;

    DrawTextWithOutline<int>
    (
        GetFont(fontQuality),
        fontSize,
        { /* X */ textMarginX, /* Y */ regionMin.y + Scale(8) },
        IM_COL32(255, 255, 255, 255),
        text,
        2,
        IM_COL32(0, 0, 0, 255)
    );

    if (icon == EButtonIcon::LBRB || icon == EButtonIcon::LTRT)
    {
        auto btnIcon = GetButtonIcon(icon == EButtonIcon::LBRB ? EButtonIcon::RB : EButtonIcon::RT);

        auto dualIconMin = alignment == EButtonAlignment::Left
            ? ImVec2(/* X */ regionMin.x + *offset + textSize.x + dualIconMarginX * 2, /* Y */ iconMin.y)
            : ImVec2(/* X */ regionMax.x - *offset + textSize.x - dualIconMarginX, /* Y */ iconMin.y);

        auto dualIconMax = ImVec2(dualIconMin.x + iconWidth, iconMax.y);

        drawList->AddImage(std::get<1>(btnIcon), dualIconMin, dualIconMax, GET_UV_COORDS(std::get<0>(btnIcon)));

        *offset += dualIconMarginX + iconWidth;
    }
}

void ButtonGuide::Init()
{
    auto& io = ImGui::GetIO();

    constexpr float FONT_SCALE = 2.0f;

    g_fntNewRodin = ImFontAtlasSnapshot::GetFont("FOT-NewRodinPro-M.otf", 24.0f * FONT_SCALE);
    g_fntNewRodinLQ = ImFontAtlasSnapshot::GetFont("FOT-NewRodinPro-M.otf", 19.0f);

    g_upIcons = LoadTexture(
        decompressZstd(g_mat_comon_x360_001, g_mat_comon_x360_001_uncompressed_size).get(),
        g_mat_comon_x360_001_uncompressed_size);

    g_upLMBIcon = LoadTexture(
        decompressZstd(g_left_mouse_button, g_left_mouse_button_uncompressed_size).get(),
        g_left_mouse_button_uncompressed_size);

    g_upStartBackIcons = LoadTexture(
        decompressZstd(g_start_back, g_start_back_uncompressed_size).get(),
        g_start_back_uncompressed_size);
}

void ButtonGuide::Draw()
{
    if (!s_isVisible)
        return;

    auto drawList = ImGui::GetForegroundDrawList();
    auto& res = ImGui::GetIO().DisplaySize;

    auto regionMarginX = Scale(g_sideMargins);
    auto regionHeight = Scale(102);

    ImVec2 regionMin = { regionMarginX, res.y - regionHeight };
    ImVec2 regionMax = { res.x - regionMarginX, res.y };

    auto textMarginX = Scale(57);
    auto textMarginY = Scale(8);
    auto iconMarginX = Scale(4);
    auto fontSize = Scale(22.5f);

    auto offsetLeft = 0.0f;
    auto offsetRight = 0.0f;

    // Draw left aligned icons.
    for (int i = 0; i < g_buttons.size(); i++)
    {
        auto& btn = g_buttons[i];

        if (btn.Alignment != EButtonAlignment::Left)
            continue;

        if (btn.Visibility && !*btn.Visibility)
            continue;

        auto iconWidth = Scale(g_iconWidths[btn.Icon]);
        auto iconHeight = Scale(g_iconHeights[btn.Icon]);
        auto textSize = g_fntNewRodin->CalcTextSizeA(fontSize, FLT_MAX, 0, btn.Name.c_str());

        if (i > 0)
            offsetLeft += textSize.x + iconWidth + textMarginX;

        ImVec2 iconMin = { regionMin.x + offsetLeft - iconWidth - iconMarginX, regionMin.y };
        ImVec2 iconMax = { regionMin.x + offsetLeft - iconMarginX, regionMin.y + iconHeight };

        DrawGuide(&offsetLeft, regionMin, regionMax, btn.Icon, btn.Alignment, iconMin, iconMax, btn.FontQuality, textSize, fontSize, btn.Name.c_str());
    }

    // Draw right aligned icons.
    for (int i = g_buttons.size() - 1; i >= 0; i--)
    {
        auto& btn = g_buttons[i];

        if (btn.Alignment != EButtonAlignment::Right)
            continue;

        if (btn.Visibility && !*btn.Visibility)
            continue;

        auto iconWidth = Scale(g_iconWidths[btn.Icon]);
        auto iconHeight = Scale(g_iconHeights[btn.Icon]);
        auto textSize = g_fntNewRodin->CalcTextSizeA(fontSize, FLT_MAX, 0, btn.Name.c_str());

        if (i < g_buttons.size() - 1)
            offsetRight += textSize.x + iconWidth + textMarginX;

        ImVec2 iconMin = { regionMax.x - offsetRight - iconWidth - iconMarginX, regionMin.y };
        ImVec2 iconMax = { regionMax.x - offsetRight - iconMarginX, regionMin.y + iconHeight };

        DrawGuide(&offsetRight, regionMin, regionMax, btn.Icon, btn.Alignment, iconMin, iconMax, btn.FontQuality, textSize, fontSize, btn.Name.c_str());
    }
}

void ButtonGuide::Open(const std::vector<Button> buttons)
{
    s_isVisible = true;
    g_sideMargins = DEFAULT_SIDE_MARGINS;
    g_buttons = buttons;
}

void ButtonGuide::SetSideMargins(float width = DEFAULT_SIDE_MARGINS)
{
    g_sideMargins = width;
}

void ButtonGuide::Close()
{
    s_isVisible = false;
}
