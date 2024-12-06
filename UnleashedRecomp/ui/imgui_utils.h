#pragma once

#include <gpu/imgui_common.h>
#include <gpu/video.h>
#include <app.h>

#define PIXELS_TO_UV_COORDS(textureWidth, textureHeight, x, y, width, height) \
    std::make_tuple(ImVec2((float)x / (float)textureWidth, (float)y / (float)textureHeight), \
                    ImVec2(((float)x + (float)width) / (float)textureWidth, ((float)y + (float)height) / (float)textureHeight))

#define GET_UV_COORDS(tuple) std::get<0>(tuple), std::get<1>(tuple)

#define CENTRE_TEXT_HORZ(min, max, textSize) min.x + ((max.x - min.x) - textSize.x) / 2
#define CENTRE_TEXT_VERT(min, max, textSize) min.y + ((max.y - min.y) - textSize.y) / 2

static void SetGradient(const ImVec2& min, const ImVec2& max, ImU32 top, ImU32 bottom)
{
    auto callbackData = AddImGuiCallback(ImGuiCallback::SetGradient);
    callbackData->setGradient.gradientMin[0] = min.x;
    callbackData->setGradient.gradientMin[1] = min.y;
    callbackData->setGradient.gradientMax[0] = max.x;
    callbackData->setGradient.gradientMax[1] = max.y;
    callbackData->setGradient.gradientTop = top;
    callbackData->setGradient.gradientBottom = bottom;
}

static void ResetGradient()
{
    auto callbackData = AddImGuiCallback(ImGuiCallback::SetGradient);
    memset(&callbackData->setGradient, 0, sizeof(callbackData->setGradient));
}

static void SetShaderModifier(uint32_t shaderModifier)
{
    auto callbackData = AddImGuiCallback(ImGuiCallback::SetShaderModifier);
    callbackData->setShaderModifier.shaderModifier = shaderModifier;
}

static void SetOrigin(ImVec2 origin)
{
    auto callbackData = AddImGuiCallback(ImGuiCallback::SetOrigin);
    callbackData->setOrigin.origin[0] = origin.x;
    callbackData->setOrigin.origin[1] = origin.y;
}

static void SetScale(ImVec2 scale)
{
    auto callbackData = AddImGuiCallback(ImGuiCallback::SetScale);
    callbackData->setScale.scale[0] = scale.x;
    callbackData->setScale.scale[1] = scale.y;
}

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

static double ComputeMotion(double duration, double offset, double total)
{
    return sqrt(std::clamp((ImGui::GetTime() - duration - offset / 60.0) / total * 60.0, 0.0, 1.0));
}

static void DrawPauseContainer(GuestTexture* texture, ImVec2 min, ImVec2 max, float alpha = 1)
{
    auto drawList = ImGui::GetForegroundDrawList();

    auto commonWidth = Scale(35);
    auto commonHeight = Scale(35);
    auto bottomHeight = Scale(5);

    auto tl = PIXELS_TO_UV_COORDS(512, 512, 0, 0, 35, 35);
    auto tc = PIXELS_TO_UV_COORDS(512, 512, 51, 0, 5, 35);
    auto tr = PIXELS_TO_UV_COORDS(512, 512, 70, 0, 35, 35);
    auto cl = PIXELS_TO_UV_COORDS(512, 512, 0, 35, 35, 235);
    auto cc = PIXELS_TO_UV_COORDS(512, 512, 51, 35, 5, 235);
    auto cr = PIXELS_TO_UV_COORDS(512, 512, 70, 35, 35, 235);
    auto bl = PIXELS_TO_UV_COORDS(512, 512, 0, 270, 35, 40);
    auto bc = PIXELS_TO_UV_COORDS(512, 512, 51, 270, 5, 40);
    auto br = PIXELS_TO_UV_COORDS(512, 512, 70, 270, 35, 40);

    auto colour = IM_COL32(255, 255, 255, 255 * alpha);

    drawList->AddImage(texture, min, { min.x + commonWidth, min.y + commonHeight }, GET_UV_COORDS(tl), colour);
    drawList->AddImage(texture, { min.x + commonWidth, min.y }, { max.x - commonWidth, min.y + commonHeight }, GET_UV_COORDS(tc), colour);
    drawList->AddImage(texture, { max.x - commonWidth, min.y }, { max.x, min.y + commonHeight }, GET_UV_COORDS(tr), colour);
    drawList->AddImage(texture, { min.x, min.y + commonHeight }, { min.x + commonWidth, max.y - commonHeight }, GET_UV_COORDS(cl), colour);
    drawList->AddImage(texture, { min.x + commonWidth, min.y + commonHeight }, { max.x - commonWidth, max.y - commonHeight }, GET_UV_COORDS(cc), colour);
    drawList->AddImage(texture, { max.x - commonWidth, min.y + commonHeight }, { max.x, max.y - commonHeight }, GET_UV_COORDS(cr), colour);
    drawList->AddImage(texture, { min.x, max.y - commonHeight }, { min.x + commonWidth, max.y + bottomHeight }, GET_UV_COORDS(bl), colour);
    drawList->AddImage(texture, { min.x + commonWidth, max.y - commonHeight }, { max.x - commonWidth, max.y + bottomHeight }, GET_UV_COORDS(bc), colour);
    drawList->AddImage(texture, { max.x - commonWidth, max.y - commonHeight }, { max.x, max.y + bottomHeight }, GET_UV_COORDS(br), colour);
}

static void DrawPauseHeaderContainer(GuestTexture* texture, ImVec2 min, ImVec2 max, float alpha = 1)
{
    auto drawList = ImGui::GetForegroundDrawList();

    auto commonWidth = Scale(35);

    auto left = PIXELS_TO_UV_COORDS(512, 512, 0, 314, 35, 60);
    auto centre = PIXELS_TO_UV_COORDS(512, 512, 51, 314, 5, 60);
    auto right = PIXELS_TO_UV_COORDS(512, 512, 70, 314, 35, 60);

    auto colour = IM_COL32(255, 255, 255, 255 * alpha);

    drawList->AddImage(texture, min, { min.x + commonWidth, max.y }, GET_UV_COORDS(left), colour);
    drawList->AddImage(texture, { min.x + commonWidth, min.y }, { max.x - commonWidth, max.y }, GET_UV_COORDS(centre), colour);
    drawList->AddImage(texture, { max.x - commonWidth, min.y }, max, GET_UV_COORDS(right), colour);
}

static void DrawTextWithMarquee(const ImFont* font, float fontSize, const ImVec2& pos, const ImVec2& min, const ImVec2& max, ImU32 color, const char* text, double time, double delay, double speed)
{
    auto drawList = ImGui::GetForegroundDrawList();
    auto rectWidth = max.x - min.x;
    auto textSize = font->CalcTextSizeA(fontSize, FLT_MAX, 0, text);
    auto textX = pos.x - fmodf(std::max(0.0, ImGui::GetTime() - (time + delay)) * speed, textSize.x + rectWidth);

    drawList->PushClipRect(min, max, true);

    if (textX <= pos.x)
        drawList->AddText(font, fontSize, { textX, pos.y }, color, text);

    if (textX + textSize.x < pos.x)
        drawList->AddText(font, fontSize, { textX + textSize.x + rectWidth, pos.y }, color, text);

    drawList->PopClipRect();
}

template<typename T>
static void DrawTextWithOutline(const ImFont* font, float fontSize, const ImVec2& pos, ImU32 color, const char* text, T outlineSize, ImU32 outlineColor)
{
    auto drawList = ImGui::GetForegroundDrawList();

    outlineSize = Scale(outlineSize);

    if constexpr (std::is_same_v<float, T> || std::is_same_v<double, T>)
    {
        // TODO: This is still very inefficient!
        for (float i = -outlineSize; i <= outlineSize; i += 0.5f)
        {
            for (float j = -outlineSize; j <= outlineSize; j += 0.5f)
            {
                if (i == 0.0f && j == 0.0f)
                    continue;

                drawList->AddText(font, fontSize, { pos.x + i, pos.y + j }, outlineColor, text);
            }
        }
    }
    else if constexpr (std::is_integral_v<T>)
    {
        // TODO: This is very inefficient!
        for (int32_t i = -outlineSize + 1; i < outlineSize; i++)
        {
            for (int32_t j = -outlineSize + 1; j < outlineSize; j++)
                drawList->AddText(font, fontSize, { pos.x + i, pos.y + j }, outlineColor, text);
        }
    }

    drawList->AddText(font, fontSize, pos, color, text);
}

static void DrawTextWithShadow(const ImFont* font, float fontSize, const ImVec2& pos, ImU32 colour, const char* text, float offset = 2.0f, float radius = 0.4f, ImU32 shadowColour = IM_COL32(0, 0, 0, 255))
{
    auto drawList = ImGui::GetForegroundDrawList();

    offset = Scale(offset);
    radius = Scale(radius);

    DrawTextWithOutline<float>(font, fontSize, { pos.x + offset, pos.y + offset }, shadowColour, text, radius, shadowColour);

    drawList->AddText(font, fontSize, pos, colour, text, nullptr);
}

static float CalcWidestTextSize(const ImFont* font, float fontSize, std::span<std::string> strs)
{
    auto result = 0.0f;

    for (auto& str : strs)
        result = std::max(result, font->CalcTextSizeA(fontSize, FLT_MAX, 0, str.c_str()).x);

    return result;
}

static std::string Truncate(const std::string& input, size_t maxLength, bool useEllipsis = true, bool usePrefixEllipsis = false)
{
    const std::string ellipsis = "...";

    if (input.length() > maxLength)
    {
        if (useEllipsis && maxLength > ellipsis.length())
        {
            if (usePrefixEllipsis)
            {
                return ellipsis + input.substr(0, maxLength - ellipsis.length());
            }
            else
            {
                return input.substr(0, maxLength - ellipsis.length()) + ellipsis;
            }
        }
        else
        {
            return input.substr(0, maxLength);
        }
    }

    return input;
}

static std::vector<std::string> Split(const char* str, char delimiter)
{
    std::vector<std::string> result;

    if (!str)
        return result;

    const char* start = str;
    const char* current = str;

    while (*current)
    {
        if (*current == delimiter)
        {
            result.emplace_back(start, current - start);
            start = current + 1;
        }

        current++;
    }

    result.emplace_back(start);

    return result;
}

static ImVec2 MeasureCentredParagraph(const ImFont* font, float fontSize, float lineMargin, std::vector<std::string> lines)
{
    auto x = 0.0f;
    auto y = 0.0f;

    for (auto& str : lines)
    {
        auto textSize = font->CalcTextSizeA(fontSize, FLT_MAX, 0, str.c_str());

        x = std::max(x, textSize.x);
        y += textSize.y + Scale(lineMargin);
    }

    return { x, y };
}

static ImVec2 MeasureCentredParagraph(const ImFont* font, float fontSize, float lineMargin, const char* text)
{
    return MeasureCentredParagraph(font, fontSize, lineMargin, Split(text, '\n'));
}

static void DrawCentredParagraph(const ImFont* font, float fontSize, const ImVec2& centre, float lineMargin, const char* text, std::function<void(const char*, ImVec2)> drawMethod)
{
    auto lines = Split(text, '\n');
    auto paragraphSize = MeasureCentredParagraph(font, fontSize, lineMargin, lines);
    auto offsetY = 0.0f;

    auto hasList = std::strstr(text, "- ");
    auto isList = false;
    auto listOffsetX = 0.0f;

    for (int i = 0; i < lines.size(); i++)
    {
        auto& str = lines[i];
        auto textSize = font->CalcTextSizeA(fontSize, FLT_MAX, 0, str.c_str());

        if (hasList)
        {
            if (!isList && str.starts_with("- ") && lines.size() > i + 1 && lines[i + 1].starts_with("- "))
            {
                isList = true;
                listOffsetX = centre.x - textSize.x / 2;
            }
            else if (isList && !str.starts_with("- "))
            {
                isList = false;
            }
        }

        drawMethod(str.c_str(), ImVec2(/* X */ isList ? listOffsetX : centre.x - textSize.x / 2, /* Y */ centre.y - paragraphSize.y / 2 + offsetY));

        offsetY += textSize.y + Scale(lineMargin);
    }
}

static void DrawTextWithMarqueeShadow(const ImFont* font, float fontSize, const ImVec2& pos, const ImVec2& min, const ImVec2& max, ImU32 colour, const char* text, double time, double delay, double speed, float offset = 2.0f, float radius = 0.4f, ImU32 shadowColour = IM_COL32(0, 0, 0, 255))
{
    auto drawList = ImGui::GetForegroundDrawList();
    auto rectWidth = max.x - min.x;
    auto textSize = font->CalcTextSizeA(fontSize, FLT_MAX, 0, text);
    auto textX = pos.x - fmodf(std::max(0.0, ImGui::GetTime() - (time + delay)) * speed, textSize.x + rectWidth);

    drawList->PushClipRect(min, max, true);

    if (textX <= pos.x)
        DrawTextWithShadow(font, fontSize, { textX, pos.y }, colour, text, offset, radius, shadowColour);

    if (textX + textSize.x < pos.x)
        DrawTextWithShadow(font, fontSize, { textX + textSize.x + rectWidth, pos.y }, colour, text, offset, radius, shadowColour);

    drawList->PopClipRect();
}

static float Lerp(float a, float b, float t)
{
    return a + (b - a) * t;
}

static float Cubic(float a, float b, float t)
{
    return a + (b - a) * (t * t * t);
}

static float Hermite(float a, float b, float t)
{
    return a + (b - a) * (t * t * (3 - 2 * t));
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
