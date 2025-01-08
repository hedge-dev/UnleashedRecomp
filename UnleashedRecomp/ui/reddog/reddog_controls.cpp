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

    ImVec2 size = { BUTTON_SIZE + labelSize.x, BUTTON_SIZE };
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

    auto left = PIXELS_TO_UV_COORDS(32, 32, 0, 0, 9, 32);
    auto centre = PIXELS_TO_UV_COORDS(32, 32, 9, 0, 14, 32);
    auto right = PIXELS_TO_UV_COORDS(32, 32, 23, 0, 9, 32);

    window->DrawList->AddImage(texture, rect.Min, { rect.Min.x + 9, rect.Max.y }, GET_UV_COORDS(left));
    window->DrawList->AddImage(texture, { rect.Min.x + 9, rect.Min.y }, { rect.Max.x - 9, rect.Max.y }, GET_UV_COORDS(centre));
    window->DrawList->AddImage(texture, { rect.Max.x - 9, rect.Min.y }, rect.Max, GET_UV_COORDS(right));

    if (label && label[0] != '\0')
        ImGui::RenderText({ rect.Min.x + (size.x - labelSize.x) * 0.5f, rect.Min.y + (size.y - labelSize.y) * 0.5f }, label);

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
