#pragma once

struct TVStatic
{
    static void Init();
    static float ComputeThumbnailAlpha(double appearTime);
    static void Draw(const ImVec2& center, const ImVec2& resolution, double appearTime);
};
