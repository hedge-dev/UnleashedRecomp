#include "imgui_utils.h"
#include <patches/aspect_ratio_patches.h>
#include <app.h>
#include <decompressor.h>
#include <version.h>

#include <res/images/common/general_window.dds.h>
#include <res/images/common/light.dds.h>
#include <res/images/common/select_fade.dds.h>
#include <res/images/common/select_fill.dds.h>

std::unique_ptr<GuestTexture> g_texGeneralWindow;
std::unique_ptr<GuestTexture> g_texLight;
std::unique_ptr<GuestTexture> g_texSelectFade;
std::unique_ptr<GuestTexture> g_texSelectFill;

void InitImGuiUtils()
{
    g_texGeneralWindow = LOAD_ZSTD_TEXTURE(g_general_window);
    g_texLight = LOAD_ZSTD_TEXTURE(g_light);
    g_texSelectFade = LOAD_ZSTD_TEXTURE(g_select_fade);
    g_texSelectFill = LOAD_ZSTD_TEXTURE(g_select_fill);
}

void SetGradient(const ImVec2& min, const ImVec2& max, ImU32 top, ImU32 bottom)
{
    auto callbackData = AddImGuiCallback(ImGuiCallback::SetGradient);
    callbackData->setGradient.boundsMin[0] = min.x;
    callbackData->setGradient.boundsMin[1] = min.y;
    callbackData->setGradient.boundsMax[0] = max.x;
    callbackData->setGradient.boundsMax[1] = max.y;
    callbackData->setGradient.gradientTop = top;
    callbackData->setGradient.gradientBottom = bottom;
}

void ResetGradient()
{
    auto callbackData = AddImGuiCallback(ImGuiCallback::SetGradient);
    memset(&callbackData->setGradient, 0, sizeof(callbackData->setGradient));
}

void SetShaderModifier(uint32_t shaderModifier)
{
    auto callbackData = AddImGuiCallback(ImGuiCallback::SetShaderModifier);
    callbackData->setShaderModifier.shaderModifier = shaderModifier;
}

void SetOrigin(ImVec2 origin)
{
    auto callbackData = AddImGuiCallback(ImGuiCallback::SetOrigin);
    callbackData->setOrigin.origin[0] = origin.x;
    callbackData->setOrigin.origin[1] = origin.y;
}

void SetScale(ImVec2 scale)
{
    auto callbackData = AddImGuiCallback(ImGuiCallback::SetScale);
    callbackData->setScale.scale[0] = scale.x;
    callbackData->setScale.scale[1] = scale.y;
}

void SetTextSkew(float yCenter, float skewScale)
{
    SetShaderModifier(IMGUI_SHADER_MODIFIER_TEXT_SKEW);
    SetOrigin({ 0.0f, yCenter });
    SetScale({ skewScale, 1.0f });
}

void ResetTextSkew()
{
    SetShaderModifier(IMGUI_SHADER_MODIFIER_NONE);
    SetOrigin({ 0.0f, 0.0f });
    SetScale({ 1.0f, 1.0f });
}

void SetMarqueeFade(ImVec2 min, ImVec2 max, float fadeScale)
{
    auto callbackData = AddImGuiCallback(ImGuiCallback::SetMarqueeFade);
    callbackData->setMarqueeFade.boundsMin[0] = min.x;
    callbackData->setMarqueeFade.boundsMin[1] = min.y;
    callbackData->setMarqueeFade.boundsMax[0] = max.x;
    callbackData->setMarqueeFade.boundsMax[1] = max.y;

    SetShaderModifier(IMGUI_SHADER_MODIFIER_MARQUEE_FADE);
    SetScale({ fadeScale, 1.0f });
}

void ResetMarqueeFade()
{
    ResetGradient();
    SetShaderModifier(IMGUI_SHADER_MODIFIER_NONE);
    SetScale({ 1.0f, 1.0f });
}

void SetOutline(float outline)
{
    auto callbackData = AddImGuiCallback(ImGuiCallback::SetOutline);
    callbackData->setOutline.outline = outline;
}

void ResetOutline()
{
    SetOutline(0.0f);
}

void SetProceduralOrigin(ImVec2 proceduralOrigin)
{
    auto callbackData = AddImGuiCallback(ImGuiCallback::SetProceduralOrigin);
    callbackData->setProceduralOrigin.proceduralOrigin[0] = proceduralOrigin.x;
    callbackData->setProceduralOrigin.proceduralOrigin[1] = proceduralOrigin.y;
}

void ResetProceduralOrigin()
{
    SetProceduralOrigin({ 0.0f, 0.0f });
}

void SetAdditive(bool enabled)
{
    auto callbackData = AddImGuiCallback(ImGuiCallback::SetAdditive);
    callbackData->setAdditive.enabled = enabled;
}

void ResetAdditive()
{
    SetAdditive(false);
}

float Scale(float size)
{
    return size * g_aspectRatioScale;
}

double ComputeLinearMotion(double duration, double offset, double total)
{
    return std::clamp((ImGui::GetTime() - duration - offset / 60.0) / total * 60.0, 0.0, 1.0);
}

double ComputeMotion(double duration, double offset, double total)
{
    return sqrt(ComputeLinearMotion(duration, offset, total));
}

void DrawPauseContainer(ImVec2 min, ImVec2 max, float alpha)
{
    auto drawList = ImGui::GetForegroundDrawList();

    auto commonWidth = Scale(35);
    auto commonHeight = Scale(35);
    auto bottomHeight = Scale(5);

    auto tl = PIXELS_TO_UV_COORDS(128, 512, 0, 0, 35, 35);
    auto tc = PIXELS_TO_UV_COORDS(128, 512, 51, 0, 5, 35);
    auto tr = PIXELS_TO_UV_COORDS(128, 512, 70, 0, 35, 35);
    auto cl = PIXELS_TO_UV_COORDS(128, 512, 0, 35, 35, 235);
    auto cc = PIXELS_TO_UV_COORDS(128, 512, 51, 35, 5, 235);
    auto cr = PIXELS_TO_UV_COORDS(128, 512, 70, 35, 35, 235);
    auto bl = PIXELS_TO_UV_COORDS(128, 512, 0, 270, 35, 40);
    auto bc = PIXELS_TO_UV_COORDS(128, 512, 51, 270, 5, 40);
    auto br = PIXELS_TO_UV_COORDS(128, 512, 70, 270, 35, 40);

    auto colour = IM_COL32(255, 255, 255, 255 * alpha);

    drawList->AddImage(g_texGeneralWindow.get(), min, { min.x + commonWidth, min.y + commonHeight }, GET_UV_COORDS(tl), colour);
    drawList->AddImage(g_texGeneralWindow.get(), { min.x + commonWidth, min.y }, { max.x - commonWidth, min.y + commonHeight }, GET_UV_COORDS(tc), colour);
    drawList->AddImage(g_texGeneralWindow.get(), { max.x - commonWidth, min.y }, { max.x, min.y + commonHeight }, GET_UV_COORDS(tr), colour);
    drawList->AddImage(g_texGeneralWindow.get(), { min.x, min.y + commonHeight }, { min.x + commonWidth, max.y - commonHeight }, GET_UV_COORDS(cl), colour);
    drawList->AddImage(g_texGeneralWindow.get(), { min.x + commonWidth, min.y + commonHeight }, { max.x - commonWidth, max.y - commonHeight }, GET_UV_COORDS(cc), colour);
    drawList->AddImage(g_texGeneralWindow.get(), { max.x - commonWidth, min.y + commonHeight }, { max.x, max.y - commonHeight }, GET_UV_COORDS(cr), colour);
    drawList->AddImage(g_texGeneralWindow.get(), { min.x, max.y - commonHeight }, { min.x + commonWidth, max.y + bottomHeight }, GET_UV_COORDS(bl), colour);
    drawList->AddImage(g_texGeneralWindow.get(), { min.x + commonWidth, max.y - commonHeight }, { max.x - commonWidth, max.y + bottomHeight }, GET_UV_COORDS(bc), colour);
    drawList->AddImage(g_texGeneralWindow.get(), { max.x - commonWidth, max.y - commonHeight }, { max.x, max.y + bottomHeight }, GET_UV_COORDS(br), colour);
}

void DrawPauseHeaderContainer(ImVec2 min, ImVec2 max, float alpha)
{
    auto drawList = ImGui::GetForegroundDrawList();

    auto commonWidth = Scale(35);

    auto left = PIXELS_TO_UV_COORDS(128, 512, 0, 314, 35, 60);
    auto centre = PIXELS_TO_UV_COORDS(128, 512, 51, 314, 5, 60);
    auto right = PIXELS_TO_UV_COORDS(128, 512, 70, 314, 35, 60);

    auto colour = IM_COL32(255, 255, 255, 255 * alpha);

    drawList->AddImage(g_texGeneralWindow.get(), min, { min.x + commonWidth, max.y }, GET_UV_COORDS(left), colour);
    drawList->AddImage(g_texGeneralWindow.get(), { min.x + commonWidth, min.y }, { max.x - commonWidth, max.y }, GET_UV_COORDS(centre), colour);
    drawList->AddImage(g_texGeneralWindow.get(), { max.x - commonWidth, min.y }, max, GET_UV_COORDS(right), colour);
}

void DrawTextWithMarquee(const ImFont* font, float fontSize, const ImVec2& pos, const ImVec2& min, const ImVec2& max, ImU32 color, const char* text, double time, double delay, double speed)
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

void DrawTextWithOutline(const ImFont* font, float fontSize, const ImVec2& pos, ImU32 color, const char* text, float outlineSize, ImU32 outlineColor, uint32_t shaderModifier)
{
    auto drawList = ImGui::GetForegroundDrawList();

    SetOutline(outlineSize);
    drawList->AddText(font, fontSize, pos, outlineColor, text);
    ResetOutline();

    if (shaderModifier != IMGUI_SHADER_MODIFIER_NONE)
        SetShaderModifier(shaderModifier);

    drawList->AddText(font, fontSize, pos, color, text);

    if (shaderModifier != IMGUI_SHADER_MODIFIER_NONE)
        SetShaderModifier(IMGUI_SHADER_MODIFIER_NONE);
}

void DrawTextWithShadow(const ImFont* font, float fontSize, const ImVec2& pos, ImU32 colour, const char* text, float offset, float radius, ImU32 shadowColour)
{
    auto drawList = ImGui::GetForegroundDrawList();

    offset = Scale(offset);

    SetOutline(radius);
    drawList->AddText(font, fontSize, { pos.x + offset, pos.y + offset }, shadowColour, text);
    ResetOutline();

    drawList->AddText(font, fontSize, pos, colour, text, nullptr);
}

float CalcWidestTextSize(const ImFont* font, float fontSize, std::span<std::string> strs)
{
    auto result = 0.0f;

    for (auto& str : strs)
        result = std::max(result, font->CalcTextSizeA(fontSize, FLT_MAX, 0, str.c_str()).x);

    return result;
}

std::string Truncate(const std::string& input, size_t maxLength, bool useEllipsis, bool usePrefixEllipsis)
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

std::vector<std::string> Split(const char* strStart, const ImFont *font, float fontSize, float maxWidth)
{
    if (!strStart)
        return {};

    std::vector<std::string> result;
    float textWidth = 0.0f;
    float lineWidth = 0.0f;
    const float scale = fontSize / font->FontSize;
    const char *str = strStart;
    const char *strEnd = strStart + strlen(strStart);
    const char *lineStart = strStart;
    const bool wordWrapEnabled = (maxWidth > 0.0f);
    const char *wordWrapEOL = nullptr;
    while (*str != 0) 
    {
        if (wordWrapEnabled)
        {
            if (wordWrapEOL == nullptr)
            {
                wordWrapEOL = font->CalcWordWrapPositionA(scale, str, strEnd, maxWidth - lineWidth);
            }

            if (str >= wordWrapEOL)
            {
                if (textWidth < lineWidth)
                    textWidth = lineWidth;

                result.emplace_back(lineStart, str);
                lineWidth = 0.0f;
                wordWrapEOL = nullptr;

                while (str < strEnd && ImCharIsBlankA(*str))
                    str++;

                if (*str == '\n')
                    str++;

                lineStart = str;
                continue;
            }
        }

        const char *prevStr = str;
        unsigned int c = (unsigned int)*str;
        if (c < 0x80)
            str += 1;
        else
            str += ImTextCharFromUtf8(&c, str, strEnd);

        if (c < 32)
        {
            if (c == '\n')
            {
                result.emplace_back(lineStart, str - 1);
                lineStart = str;
                textWidth = ImMax(textWidth, lineWidth);
                lineWidth = 0.0f;
                continue;
            }

            if (c == '\r')
            {
                lineStart = str;
                continue;
            }
        }

        const float charWidth = ((int)c < font->IndexAdvanceX.Size ? font->IndexAdvanceX.Data[c] : font->FallbackAdvanceX) * scale;
        if (lineWidth + charWidth >= maxWidth)
        {
            str = prevStr;
            break;
        }

        lineWidth += charWidth;
    }

    if (str != lineStart) 
    {
        result.emplace_back(lineStart, str);
    }

    return result;
}

ImVec2 MeasureCentredParagraph(const ImFont* font, float fontSize, float lineMargin, std::vector<std::string> lines)
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

ImVec2 MeasureCentredParagraph(const ImFont* font, float fontSize, float maxWidth, float lineMargin, const char* text)
{
    return MeasureCentredParagraph(font, fontSize, lineMargin, Split(text, font, fontSize, maxWidth));
}

void DrawCentredParagraph(const ImFont* font, float fontSize, float maxWidth, const ImVec2& centre, float lineMargin, const char* text, std::function<void(const char*, ImVec2)> drawMethod)
{
    auto lines = Split(text, font, fontSize, maxWidth);
    auto paragraphSize = MeasureCentredParagraph(font, fontSize, lineMargin, lines);
    auto offsetY = 0.0f;

    for (int i = 0; i < lines.size(); i++)
    {
        auto& str = lines[i];
        auto textSize = font->CalcTextSizeA(fontSize, FLT_MAX, 0, str.c_str());

        auto textX = str.starts_with("- ")
            ? centre.x - paragraphSize.x / 2
            : centre.x - textSize.x / 2;

        auto textY = centre.y - paragraphSize.y / 2 + offsetY;

        drawMethod(str.c_str(), { textX, textY });

        offsetY += textSize.y + Scale(lineMargin);
    }
}

void DrawTextWithMarqueeShadow(const ImFont* font, float fontSize, const ImVec2& pos, const ImVec2& min, const ImVec2& max, ImU32 colour, const char* text, double time, double delay, double speed, float offset, float radius, ImU32 shadowColour)
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

float Lerp(float a, float b, float t)
{
    return a + (b - a) * t;
}

float Cubic(float a, float b, float t)
{
    return a + (b - a) * (t * t * t);
}

float Hermite(float a, float b, float t)
{
    return a + (b - a) * (t * t * (3 - 2 * t));
}

ImVec2 Lerp(const ImVec2& a, const ImVec2& b, float t)
{
    return { a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t };
}

ImU32 ColourLerp(ImU32 c0, ImU32 c1, float t)
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

void DrawVersionString(const ImFont* font, const ImU32 col)
{
    auto drawList = ImGui::GetForegroundDrawList();
    auto& res = ImGui::GetIO().DisplaySize;
    auto fontSize = Scale(12);
    auto textMargin = Scale(2);
    auto textSize = font->CalcTextSizeA(fontSize, FLT_MAX, 0, g_versionString);

    drawList->AddText(font, fontSize, { res.x - textSize.x - textMargin, res.y - textSize.y - textMargin }, col, g_versionString);
}

void DrawSelectionContainer(ImVec2 min, ImVec2 max, bool fadeTop)
{
    auto drawList = ImGui::GetForegroundDrawList();

    static auto breatheStart = ImGui::GetTime();
    auto alpha = BREATHE_MOTION(1.0f, 0.55f, breatheStart, 0.92f);
    auto colour = IM_COL32(255, 255, 255, 255 * alpha);
    auto commonWidth = Scale(11);
    auto commonHeight = Scale(24);

    if (fadeTop)
    {
        auto left = PIXELS_TO_UV_COORDS(64, 64, 0, 0, 11, 50);
        auto centre = PIXELS_TO_UV_COORDS(64, 64, 11, 0, 8, 50);
        auto right = PIXELS_TO_UV_COORDS(64, 64, 19, 0, 11, 50);

        drawList->AddImage(g_texSelectFade.get(), min, { min.x + commonWidth, max.y }, GET_UV_COORDS(left), colour);
        drawList->AddImage(g_texSelectFade.get(), { min.x + commonWidth, min.y }, { max.x - commonWidth, max.y }, GET_UV_COORDS(centre), colour);
        drawList->AddImage(g_texSelectFade.get(), { max.x - commonWidth, min.y }, max, GET_UV_COORDS(right), colour);

        return;
    }

    auto tl = PIXELS_TO_UV_COORDS(64, 64, 0, 0, 11, 24);
    auto tc = PIXELS_TO_UV_COORDS(64, 64, 11, 0, 8, 24);
    auto tr = PIXELS_TO_UV_COORDS(64, 64, 19, 0, 11, 24);
    auto cl = PIXELS_TO_UV_COORDS(64, 64, 0, 24, 11, 2);
    auto cc = PIXELS_TO_UV_COORDS(64, 64, 11, 24, 8, 2);
    auto cr = PIXELS_TO_UV_COORDS(64, 64, 19, 24, 11, 2);
    auto bl = PIXELS_TO_UV_COORDS(64, 64, 0, 26, 11, 24);
    auto bc = PIXELS_TO_UV_COORDS(64, 64, 11, 26, 8, 24);
    auto br = PIXELS_TO_UV_COORDS(64, 64, 19, 26, 11, 24);

    drawList->AddImage(g_texSelectFill.get(), min, { min.x + commonWidth, min.y + commonHeight }, GET_UV_COORDS(tl), colour);
    drawList->AddImage(g_texSelectFill.get(), { min.x + commonWidth, min.y }, { max.x - commonWidth, min.y + commonHeight }, GET_UV_COORDS(tc), colour);
    drawList->AddImage(g_texSelectFill.get(), { max.x - commonWidth, min.y }, { max.x, min.y + commonHeight }, GET_UV_COORDS(tr), colour);
    drawList->AddImage(g_texSelectFill.get(), { min.x, min.y + commonHeight }, { min.x + commonWidth, max.y - commonHeight }, GET_UV_COORDS(cl), colour);
    drawList->AddImage(g_texSelectFill.get(), { min.x + commonWidth, min.y + commonHeight }, { max.x - commonWidth, max.y - commonHeight }, GET_UV_COORDS(cc), colour);
    drawList->AddImage(g_texSelectFill.get(), { max.x - commonWidth, min.y + commonHeight }, { max.x, max.y - commonHeight }, GET_UV_COORDS(cr), colour);
    drawList->AddImage(g_texSelectFill.get(), { min.x, max.y - commonHeight }, { min.x + commonWidth, max.y }, GET_UV_COORDS(bl), colour);
    drawList->AddImage(g_texSelectFill.get(), { min.x + commonWidth, max.y - commonHeight }, { max.x - commonWidth, max.y }, GET_UV_COORDS(bc), colour);
    drawList->AddImage(g_texSelectFill.get(), { max.x - commonWidth, max.y - commonHeight }, { max.x, max.y }, GET_UV_COORDS(br), colour);
}

void DrawToggleLight(ImVec2 pos, bool isEnabled)
{
    auto drawList = ImGui::GetForegroundDrawList();
    auto lightSize = Scale(14);

    ImVec2 min = { pos.x, pos.y };
    ImVec2 max = { min.x + lightSize, min.y + lightSize };

    if (isEnabled)
    {
        auto lightGlowSize = Scale(24);
        auto lightGlowUVs = PIXELS_TO_UV_COORDS(64, 64, 31, 31, 32, 32);

        ImVec2 lightGlowMin = { min.x - (lightGlowSize / 2) + Scale(2), min.y - lightGlowSize / 2 };
        ImVec2 lightGlowMax = { min.x + lightGlowSize, min.y + lightGlowSize };

        SetAdditive(true);
        drawList->AddImage(g_texLight.get(), lightGlowMin, lightGlowMax, GET_UV_COORDS(lightGlowUVs), IM_COL32(255, 255, 0, 127));
        SetAdditive(false);

        auto lightOnUVs = PIXELS_TO_UV_COORDS(64, 64, 14, 0, 14, 14);

        drawList->AddImage(g_texLight.get(), min, max, GET_UV_COORDS(lightOnUVs));
    }
    else
    {
        auto lightOffUVs = PIXELS_TO_UV_COORDS(64, 64, 0, 0, 14, 14);

        drawList->AddImage(g_texLight.get(), min, max, GET_UV_COORDS(lightOffUVs));
    }
}
