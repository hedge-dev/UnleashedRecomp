#include "reddog_controls.h"
#include <gpu/video.h>

#include <res/images/reddog/checkbox_1.dds.h>
#include <res/images/reddog/checkbox_2.dds.h>

constexpr float CHECKBOX_SIZE = 16.0f;

static std::unique_ptr<GuestTexture> g_upCheckbox1;
static std::unique_ptr<GuestTexture> g_upCheckbox2;

void Reddog::InitControlsResources()
{
    g_upCheckbox1 = LoadTexture(g_checkbox_1, sizeof(g_checkbox_1));
    g_upCheckbox2 = LoadTexture(g_checkbox_2, sizeof(g_checkbox_2));
}

bool Reddog::Checkbox(const char* label, bool* v)
{
    ImGuiWindow* window = ImGui::GetCurrentWindow();

    if (window->SkipItems)
        return false;

    auto& ctx = *GImGui;
    auto& style = ctx.Style;
    auto id = window->GetID(label);
    auto labelSize = ImGui::CalcTextSize(label, nullptr, true);
    auto pos = window->DC.CursorPos;

    ImVec2 size = { CHECKBOX_SIZE, CHECKBOX_SIZE };
    ImRect checkBoundingBox(pos, ImVec2(pos.x + size.x, pos.y + size.y));
    ImRect totalBoundingBox(pos, ImVec2(pos.x + size.x + (labelSize.x > 0.0f ? style.ItemInnerSpacing.x + labelSize.x : 0.0f), pos.y + size.y));

    ImGui::ItemSize(totalBoundingBox, style.FramePadding.y);

    if (!ImGui::ItemAdd(totalBoundingBox, id))
        return false;

    bool isHovered, isHeld;
    bool isPressed = ImGui::ButtonBehavior(checkBoundingBox, id, &isHovered, &isHeld);

    if (isPressed)
    {
        *v = !*v;
        ImGui::MarkItemEdited(id);
    }

    auto texture = *v
        ? g_upCheckbox2.get()
        : g_upCheckbox1.get();

    window->DrawList->AddImage(texture, checkBoundingBox.Min, checkBoundingBox.Max);

    if (labelSize.x > 0.0f)
        ImGui::RenderText({ checkBoundingBox.Max.x + style.ItemInnerSpacing.x, checkBoundingBox.Min.y + style.FramePadding.y }, label);

    return isPressed;
}
