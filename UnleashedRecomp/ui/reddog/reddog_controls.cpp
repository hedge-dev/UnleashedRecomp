#include "reddog_controls.h"
#include <gpu/video.h>
#include <ui/imgui_utils.h>

#include <res/images/reddog/checkbox_1.dds.h>
#include <res/images/reddog/checkbox_2.dds.h>
#include <res/images/reddog/common_button_1.dds.h>
#include <res/images/reddog/common_button_2.dds.h>

constexpr float BUTTON_SIZE = 32.0f;
constexpr float CHECKBOX_SIZE = 16.0f;

static std::unique_ptr<GuestTexture> g_upCheckbox1;
static std::unique_ptr<GuestTexture> g_upCheckbox2;
static std::unique_ptr<GuestTexture> g_upCommonButton1;
static std::unique_ptr<GuestTexture> g_upCommonButton2;

void Reddog::InitControlsResources()
{
    g_upCheckbox1 = LoadTexture(g_checkbox_1, sizeof(g_checkbox_1));
    g_upCheckbox2 = LoadTexture(g_checkbox_2, sizeof(g_checkbox_2));
    g_upCommonButton1 = LoadTexture(g_common_button_1, sizeof(g_common_button_1));
    g_upCommonButton2 = LoadTexture(g_common_button_2, sizeof(g_common_button_2));
}

bool Reddog::Button(const char* label)
{
    auto window = ImGui::GetCurrentWindow();

    if (window->SkipItems)
        return false;

    auto& ctx = *GImGui;
    auto& style = ctx.Style;
    auto id = window->GetID(label);
    auto pos = window->DC.CursorPos;

    ImVec2 labelSize = { 0, 0 };

    if (label && label[0] != '\0')
        labelSize = ImGui::CalcTextSize(label, nullptr, true);

    ImVec2 size = { BUTTON_SIZE / 2 + labelSize.x, BUTTON_SIZE / 2 };
    ImRect rect(pos, { pos.x + size.x, pos.y + size.y });
    ImRect paddedRect({ rect.Min.x, rect.Min.y - 1 }, { rect.Max.x, rect.Max.y + 1 });

    ImGui::ItemSize(paddedRect, style.FramePadding.y);

    if (!ImGui::ItemAdd(rect, id))
        return false;

    bool isHovered, isHeld;
    bool isPressed = ImGui::ButtonBehavior(rect, id, &isHovered, &isHeld);

    auto texture = isHeld && isHovered
        ? g_upCommonButton2.get()
        : g_upCommonButton1.get();

    constexpr float uvSize = 6.0f / 2.0f;
    auto tl = PIXELS_TO_UV_COORDS(32, 32, 0, 0, 8, 8);
    auto tc = PIXELS_TO_UV_COORDS(32, 32, 8, 0, 16, 8);
    auto tr = PIXELS_TO_UV_COORDS(32, 32, 24, 0, 8, 8);
    auto cl = PIXELS_TO_UV_COORDS(32, 32, 0, 8, 8, 16);
    auto cc = PIXELS_TO_UV_COORDS(32, 32, 8, 8, 16, 16);
    auto cr = PIXELS_TO_UV_COORDS(32, 32, 24, 8, 8, 16);
    auto bl = PIXELS_TO_UV_COORDS(32, 32, 0, 24, 8, 8);
    auto bc = PIXELS_TO_UV_COORDS(32, 32, 8, 24, 16, 8);
    auto br = PIXELS_TO_UV_COORDS(32, 32, 24, 24, 8, 8);

    window->DrawList->AddImage(texture, rect.Min, { rect.Min.x + uvSize, rect.Min.y + uvSize }, GET_UV_COORDS(tl));
    window->DrawList->AddImage(texture, { rect.Min.x + uvSize, rect.Min.y }, { rect.Max.x - uvSize, rect.Min.y + uvSize }, GET_UV_COORDS(tc));
    window->DrawList->AddImage(texture, { rect.Max.x - uvSize, rect.Min.y }, { rect.Max.x, rect.Min.y + uvSize }, GET_UV_COORDS(tr));
    window->DrawList->AddImage(texture, { rect.Min.x, rect.Min.y + uvSize }, { rect.Min.x + uvSize, rect.Max.y - uvSize }, GET_UV_COORDS(cl));
    window->DrawList->AddImage(texture, { rect.Min.x + uvSize, rect.Min.y + uvSize }, { rect.Max.x - uvSize, rect.Max.y - uvSize }, GET_UV_COORDS(cc));
    window->DrawList->AddImage(texture, { rect.Max.x - uvSize, rect.Min.y + uvSize }, { rect.Max.x, rect.Max.y - uvSize }, GET_UV_COORDS(cr));
    window->DrawList->AddImage(texture, { rect.Min.x, rect.Max.y - uvSize }, { rect.Min.x + uvSize, rect.Max.y }, GET_UV_COORDS(bl));
    window->DrawList->AddImage(texture, { rect.Min.x + uvSize, rect.Max.y - uvSize }, { rect.Max.x - uvSize, rect.Max.y }, GET_UV_COORDS(bc));
    window->DrawList->AddImage(texture, { rect.Max.x - uvSize, rect.Max.y - uvSize }, rect.Max, GET_UV_COORDS(br));

    float textOffset = isHeld && isPressed ? 1.0f : 0.0f;

    if (label && label[0] != '\0')
        ImGui::RenderText({ textOffset + rect.Min.x + (size.x - labelSize.x) * 0.5f, textOffset + rect.Min.y + (size.y - labelSize.y) * 0.5f }, label);

    return isPressed;
}

bool Reddog::Checkbox(const char* label, bool* v)
{
    auto window = ImGui::GetCurrentWindow();

    if (window->SkipItems)
        return false;

    auto& ctx = *GImGui;
    auto& style = ctx.Style;
    auto id = window->GetID(label);
    auto labelSize = ImGui::CalcTextSize(label, nullptr, true);
    auto pos = window->DC.CursorPos;

    ImVec2 size = { CHECKBOX_SIZE, CHECKBOX_SIZE };
    ImRect testRect(pos, { pos.x + size.x, pos.y + size.y });
    ImRect totalRect(pos, { pos.x + size.x + (labelSize.x > 0.0f ? style.ItemInnerSpacing.x + labelSize.x : 0.0f), pos.y + size.y });

    ImGui::ItemSize(totalRect, style.FramePadding.y);

    if (!ImGui::ItemAdd(totalRect, id))
        return false;

    bool isHovered, isHeld;
    bool isPressed = ImGui::ButtonBehavior(testRect, id, &isHovered, &isHeld);

    if (isPressed)
    {
        *v = !*v;
        ImGui::MarkItemEdited(id);
    }

    auto texture = *v
        ? g_upCheckbox2.get()
        : g_upCheckbox1.get();

    window->DrawList->AddImage(texture, testRect.Min, testRect.Max);

    if (labelSize.x > 0.0f)
        ImGui::RenderText({ testRect.Max.x + style.ItemInnerSpacing.x, testRect.Min.y + style.FramePadding.y }, label);

    return isPressed;
}

void Reddog::Separator(float upperPadding, float lowerPadding)
{
    ImGui::Dummy(ImVec2(0.0f, upperPadding));
    ImGui::Separator();
    ImGui::Dummy(ImVec2(0.0f, lowerPadding));
}

bool Reddog::ExplicitButton(const char* label, EButtonTextAlignment textAlignment, const ImVec2& size, float fontScale)
{
    auto window = ImGui::GetCurrentWindow();

    if (window->SkipItems)
        return false;

    auto font = ImGui::GetFont();
    font->Scale = fontScale;

    ImGui::PushFont(font);

    ImVec2 textSize = ImGui::CalcTextSize(label);
    ImVec2 pos = ImGui::GetCursorScreenPos();
    ImVec2 buttonMax = { pos.x + size.x, pos.y + size.y };

    ImGui::InvisibleButton(label, size);

    bool isHovered = ImGui::IsItemHovered();
    bool isActive = ImGui::IsItemActive();

    ImVec4 colBg = isHovered ? ImGui::GetStyleColorVec4(ImGuiCol_ButtonHovered) : ImGui::GetStyleColorVec4(ImGuiCol_Button);
    ImVec4 colBorder = ImGui::GetStyleColorVec4(ImGuiCol_Border);
    ImVec4 colText = ImGui::GetStyleColorVec4(ImGuiCol_Text);

    window->DrawList->AddRectFilled(pos, buttonMax, ImGui::ColorConvertFloat4ToU32(colBg));
    window->DrawList->AddRect(pos, buttonMax, ImGui::ColorConvertFloat4ToU32(colBorder));

    auto colTextU32 = ImGui::ColorConvertFloat4ToU32(colText);
    auto framePadding = ImGui::GetStyle().FramePadding;
    auto marginX = 8.0f;
    auto textY = (pos.y + (size.y - textSize.y) * 0.5f) - 1.0f;

    switch (textAlignment)
    {
        case EButtonTextAlignment::Left:
            window->DrawList->AddText({ pos.x + framePadding.x + marginX, textY }, colTextU32, label);
            break;

        case EButtonTextAlignment::Centre:
            window->DrawList->AddText({ pos.x + (size.x - textSize.x) * 0.5f, textY }, colTextU32, label);
            break;

        case EButtonTextAlignment::Right:
            window->DrawList->AddText({ size.x - framePadding.x - textSize.x - marginX, textY }, colTextU32, label);
            break;
    }

    ImGui::PopFont();

    return isActive;
}
