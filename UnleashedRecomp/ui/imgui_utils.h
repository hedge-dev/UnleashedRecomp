#pragma once

// Aspect ratio aware.
static float Scale(float size)
{
    auto& io = ImGui::GetIO();

    if (io.DisplaySize.x > io.DisplaySize.y)
        return size * std::max(1.0f, io.DisplaySize.y / 720.0f);
    else
        return size * std::max(1.0f, io.DisplaySize.x / 1280.0f);
}

// Not aspect ratio aware. Will stretch.
static float ScaleX(float x)
{
    auto& io = ImGui::GetIO();
    return x * io.DisplaySize.x / 1280.0f;
}

// Not aspect ratio aware. Will stretch.
static float ScaleY(float y)
{
    auto& io = ImGui::GetIO();
    return y * io.DisplaySize.y / 720.0f;
}

static void DrawTextWithMarquee(const ImFont* font, float fontSize, const ImVec2& pos, const ImVec2& min, const ImVec2& max, ImU32 color, const char* text, double time, double delay, double speed)
{
    auto drawList = ImGui::GetForegroundDrawList();
    auto rectWidth = max.x - min.x;
    auto textSize = font->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, text);
    auto textX = pos.x - fmodf(std::max(0.0, ImGui::GetTime() - (time + delay)) * speed, textSize.x + rectWidth);

    drawList->PushClipRect(min, max, true);

    if (textX <= pos.x)
        drawList->AddText(font, fontSize, { textX, pos.y }, color, text);

    if (textX + textSize.x < pos.x)
        drawList->AddText(font, fontSize, { textX + textSize.x + rectWidth, pos.y }, color, text);

    drawList->PopClipRect();
}

static void DrawTextWithOutline(const ImFont* font, float fontSize, const ImVec2& pos, ImU32 color, const char* text, int32_t outlineSize, ImU32 outlineColor)
{
    auto drawList = ImGui::GetForegroundDrawList();

    // TODO: This is very inefficient!
    for (int32_t i = -outlineSize + 1; i < outlineSize; i++)
    {
        for (int32_t j = -outlineSize + 1; j < outlineSize; j++)
            drawList->AddText(font, fontSize, { pos.x + i, pos.y + j }, outlineColor, text);
    }

    drawList->AddText(font, fontSize, pos, color, text);
}

static void DrawTextWithShadow(const ImFont* font, float fontSize, const ImVec2& pos, ImU32 colour, const char* text, float offset = 2.5f, ImU32 shadowColour = IM_COL32(0, 0, 0, 255))
{
    auto drawList = ImGui::GetForegroundDrawList();

    drawList->AddText(font, fontSize, { pos.x + offset, pos.y + offset }, shadowColour, text);
    drawList->AddText(font, fontSize, pos, colour, text);
}

static float Lerp(float a, float b, float t)
{
    return a + (b - a) * t;
}

static float CubicEase(float a, float b, float t)
{
    return a + (b - a) * (t * t * t);
}

static ImVec2 Lerp(const ImVec2& a, const ImVec2& b, float t)
{
    return { a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t };
}

static ImU32 ColourLerp(ImU32 c0, ImU32 c1, float t)
{
    auto a = ImGui::ColorConvertU32ToFloat4(c0);
    auto b = ImGui::ColorConvertU32ToFloat4(c1);

    ImVec4 result;
    result.x = a.x + (b.x - a.x) * t;
    result.y = a.y + (b.y - a.y) * t;
    result.z = a.z + (b.z - a.z) * t;
    result.w = a.w + (b.w - a.w) * t;

    return ImGui::ColorConvertFloat4ToU32(result);
}
