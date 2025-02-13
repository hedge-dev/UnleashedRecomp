#include "tv_static.h"
#include "imgui_utils.h"

#include <decompressor.h>
#include <gpu/video.h>
#include <patches/aspect_ratio_patches.h>
#include <res/images/options_menu/options_static.dds.h>
#include <res/images/options_menu/options_static_flash.dds.h>

namespace
{
    struct FloatLinear
    {
        float value;
        float time;

        static float Sample(const FloatLinear& a, const FloatLinear& b, float time)
        {
            const float t = std::clamp((time - a.time) / (b.time - a.time), 0.0f, 1.0f);
            return (b.value - a.value) * t + a.value;
        }
    };

    struct FloatHermite
    {
        float value;
        float time;
        float inTangent;
        float outTangent;

        static float Sample(const FloatHermite& a, const FloatHermite& b, float time)
        {
            const float t = std::clamp((time - a.time) / (b.time - a.time), 0.0f, 1.0f);

            float valueDelta = b.value - a.value;
            float frameDelta = b.time - a.time;

            float biasSquaric = t * t;
            float biasCubic = biasSquaric * t;

            float valueCubic = (a.outTangent + a.inTangent) * frameDelta - valueDelta * 2.0f;
            float valueSquaric = valueDelta * 3.0f - (a.inTangent * 2.0f + a.outTangent) * frameDelta;
            float valueLinear = frameDelta * a.inTangent;

            return valueCubic * biasCubic + valueSquaric * biasSquaric + valueLinear * t + a.value;
        }
    };
}

template<typename T, size_t N>
static auto Sample(const std::array<T, N>& keys, float time)
{
    T firstKey = keys[0];
    T lastKey = keys[N - 1];

    if (time < firstKey.time)
        return firstKey.value;

    if (time > lastKey.time)
        return lastKey.value;

    size_t keyIndex = 0;
    for (auto key : keys)
    {
        if (key.time >= time)
            break;

        keyIndex++;
    }

    if (keyIndex >= N)
        return keys[N - 1].value;

    return T::Sample(keys[keyIndex - 1], keys[keyIndex], time);
}

static std::unique_ptr<GuestTexture> g_flashTexture;
static std::unique_ptr<GuestTexture> g_noiseTexture;

static constexpr float FRAME_OFFSET = 65.0f;
static constexpr float FRAME_SCALE = 1.0f / 60.0f;
static constexpr float FRAME_DURATION = 32.0f;

static std::array g_flashScaleX =
{
    FloatHermite{ 0,             67 * FRAME_SCALE, 0, 0.673736f },
    FloatHermite{ 2,             70 * FRAME_SCALE, -0.003465f, -0.682543f },
    FloatHermite{ 0,             73 * FRAME_SCALE, 0, 0 },
};
static std::array g_flashScaleY =
{
    FloatHermite{ 0,             67 * FRAME_SCALE, 0, 0.67238f },
    FloatHermite{ 2,             70 * FRAME_SCALE, -0.001741f, -0.664096f },
    FloatHermite{ 0,             73 * FRAME_SCALE, 0, 0 },
};
static std::array g_flashColor =
{
    FloatLinear{  255.0f,        67 * FRAME_SCALE },
    FloatLinear{  160.0f,        70 * FRAME_SCALE },
    FloatLinear{  255.0f,        73 * FRAME_SCALE },
};
static std::array g_flashAlpha =
{
    FloatLinear{  255.0f,        70 * FRAME_SCALE },
    FloatLinear{  0.0f,          73 * FRAME_SCALE },
};
static std::array g_noiseScale =
{
    FloatLinear{  0.0f,          70 * FRAME_SCALE },
    FloatLinear{  1.0f,          75 * FRAME_SCALE },
};
static std::array g_noiseTL_gradTL =
{
    FloatLinear{  0.0f,          77 * FRAME_SCALE },
    FloatLinear{ (192 / 255.0f), 85 * FRAME_SCALE },
    FloatLinear{  0.0f,          90 * FRAME_SCALE },
};
static std::array g_noiseTL_gradBL =
{
    FloatLinear{  0.0f,          73 * FRAME_SCALE },
    FloatLinear{ (223 / 255.0f), 85 * FRAME_SCALE },
    FloatLinear{  0.0f,          94 * FRAME_SCALE },
};
static std::array g_noiseTL_gradTR =
{
    FloatLinear{  0.0f,          74 * FRAME_SCALE },
    FloatLinear{ (223 / 255.0f), 85 * FRAME_SCALE },
    FloatLinear{  0.0f,          93 * FRAME_SCALE },
};
static std::array g_noiseTL_gradBR =
{
    FloatLinear{  0.0f,          70 * FRAME_SCALE },
    FloatLinear{  1.0f,          85 * FRAME_SCALE },
    FloatLinear{  0.0f,          97 * FRAME_SCALE },
};
static std::array g_noiseTR_gradTL =
{
    FloatLinear{  0.0f,          74 * FRAME_SCALE },
    FloatLinear{ (223 / 255.0f), 85 * FRAME_SCALE },
    FloatLinear{  0.0f,          93 * FRAME_SCALE },
};
static std::array g_noiseTR_gradBL =
{
    FloatLinear{  0.0f,          70 * FRAME_SCALE },
    FloatLinear{  1.0f,          85 * FRAME_SCALE },
    FloatLinear{  0.0f,          97 * FRAME_SCALE },
};
static std::array g_noiseTR_gradTR =
{
    FloatLinear{  0.0f,          77 * FRAME_SCALE },
    FloatLinear{ (192 / 255.0f), 85 * FRAME_SCALE },
    FloatLinear{  0.0f,          90 * FRAME_SCALE },
};
static std::array g_noiseTR_gradBR =
{
    FloatLinear{  0.0f,          73 * FRAME_SCALE },
    FloatLinear{ (223 / 255.0f), 85 * FRAME_SCALE },
    FloatLinear{  0.0f,          94 * FRAME_SCALE },
};
static std::array g_noiseBL_gradTL =
{
    FloatLinear{  0.0f,          73 * FRAME_SCALE },
    FloatLinear{ (223 / 255.0f), 85 * FRAME_SCALE },
    FloatLinear{  0.0f,          94 * FRAME_SCALE },
};
static std::array g_noiseBL_gradBL =
{
    FloatLinear{  0.0f,          77 * FRAME_SCALE },
    FloatLinear{ (192 / 255.0f), 85 * FRAME_SCALE },
    FloatLinear{  0.0f,          90 * FRAME_SCALE },
};
static std::array g_noiseBL_gradTR =
{
    FloatLinear{  0.0f,          70 * FRAME_SCALE },
    FloatLinear{  1.0f,          85 * FRAME_SCALE },
    FloatLinear{  0.0f,          97 * FRAME_SCALE },
};
static std::array g_noiseBL_gradBR =
{
    FloatLinear{  0.0f,          74 * FRAME_SCALE },
    FloatLinear{ (223 / 255.0f), 85 * FRAME_SCALE },
    FloatLinear{  0.0f,          93 * FRAME_SCALE },
};
static std::array g_noiseBR_gradTL =
{
    FloatLinear{  0.0f,          70 * FRAME_SCALE },
    FloatLinear{  1.0f,          85 * FRAME_SCALE },
    FloatLinear{  0.0f,          97 * FRAME_SCALE },
};
static std::array g_noiseBR_gradBL =
{
    FloatLinear{  0.0f,          74 * FRAME_SCALE },
    FloatLinear{ (223 / 255.0f), 85 * FRAME_SCALE },
    FloatLinear{  0.0f,          93 * FRAME_SCALE },
};
static std::array g_noiseBR_gradTR =
{
    FloatLinear{  0.0f,          73 * FRAME_SCALE },
    FloatLinear{ (223 / 255.0f), 85 * FRAME_SCALE },
    FloatLinear{  0.0f,          94 * FRAME_SCALE },
};
static std::array g_noiseBR_gradBR =
{
    FloatLinear{  0.0f,          77 * FRAME_SCALE },
    FloatLinear{ (192 / 255.0f), 85 * FRAME_SCALE },
    FloatLinear{  0.0f,          90 * FRAME_SCALE },
};
static std::array g_thumbnailAlpha =
{
    FloatLinear{  0.0f,          85 * FRAME_SCALE },
    FloatLinear{  1.0f,          90 * FRAME_SCALE },
};

static std::pair<ImVec2, ImVec2> ComputeRect(const ImVec2& center, const ImVec2& scale)
{
    ImVec2 min = { center.x - scale.x / 2.0f, center.y - scale.y / 2.0f };
    ImVec2 max = { center.x + scale.x / 2.0f, center.y + scale.y / 2.0f };

    return std::make_pair(min, max);
}

static void DrawFlash(const ImVec2& center, const ImVec2& resolution, float time)
{
    auto drawList = ImGui::GetBackgroundDrawList();

    float baseScale = resolution.y / 135.0f * 100.0f;
    auto [min, max] = ComputeRect(center, { Sample(g_flashScaleX, time) * baseScale, Sample(g_flashScaleY, time) * baseScale});

    float color = Sample(g_flashColor, time);
    float alpha = Sample(g_flashAlpha, time);

    drawList->AddImage(g_flashTexture.get(), min, max, { 0.0f, 0.0f }, { 1.0f, 1.0f }, IM_COL32(color, 255, color, alpha));
}

static void PrimRectUVColorCorners(ImDrawList* This, const ImVec2& p_min, const ImVec2& p_max, const ImVec2& uv_min, const ImVec2& uv_max,
    ImU32 col_upr_left, ImU32 col_upr_right, ImU32 col_bot_right, ImU32 col_bot_left, bool reverse_order = false)
{
    ImVec2 a(p_min), c(p_max), uv_a(uv_min), uv_c(uv_max);
    ImVec2 b(c.x, a.y), d(a.x, c.y), uv_b(uv_c.x, uv_a.y), uv_d(uv_a.x, uv_c.y);

    if (reverse_order)
    {
        ImVec2 _a = a; a = b; b = _a;
        ImVec2 _c = c; c = d; d = _c;

        ImVec2 _uv_a = uv_a; uv_a = uv_b; uv_b = _uv_a;
        ImVec2 _uv_c = uv_c; uv_c = uv_d; uv_d = _uv_c;
    }

    ImDrawIdx idx = (ImDrawIdx)This->_VtxCurrentIdx;
    This->_IdxWritePtr[0] = idx; This->_IdxWritePtr[1] = (ImDrawIdx)(idx + 1); This->_IdxWritePtr[2] = (ImDrawIdx)(idx + 2);
    This->_IdxWritePtr[3] = idx; This->_IdxWritePtr[4] = (ImDrawIdx)(idx + 2); This->_IdxWritePtr[5] = (ImDrawIdx)(idx + 3);
    This->_VtxWritePtr[0].pos = a; This->_VtxWritePtr[0].uv = uv_a; This->_VtxWritePtr[0].col = reverse_order ? col_upr_right : col_upr_left;
    This->_VtxWritePtr[1].pos = b; This->_VtxWritePtr[1].uv = uv_b; This->_VtxWritePtr[1].col = reverse_order ? col_upr_left : col_upr_right;
    This->_VtxWritePtr[2].pos = c; This->_VtxWritePtr[2].uv = uv_c; This->_VtxWritePtr[2].col = reverse_order ? col_bot_left : col_bot_right;
    This->_VtxWritePtr[3].pos = d; This->_VtxWritePtr[3].uv = uv_d; This->_VtxWritePtr[3].col = reverse_order ? col_bot_right : col_bot_left;
    This->_VtxWritePtr += 4;
    This->_VtxCurrentIdx += 4;
    This->_IdxWritePtr += 6;
}

static void AddImageGradient(ImDrawList* DrawList, ImTextureID user_texture_id, const ImVec2& p_min, const ImVec2& p_max, const ImVec2& uv_min, const ImVec2& uv_max,
    ImU32 col_upr_left, ImU32 col_upr_right, ImU32 col_bot_right, ImU32 col_bot_left, bool reverse_order = false)
{
    if (((col_upr_left | col_upr_right | col_bot_right | col_bot_left) & IM_COL32_A_MASK) == 0)
        return;

    const bool push_texture_id = user_texture_id && (user_texture_id != DrawList->_CmdHeader.TextureId);
    if (push_texture_id)
        DrawList->PushTextureID(user_texture_id);

    DrawList->PrimReserve(6, 4);

    const ImVec2 uv = DrawList->_Data->TexUvWhitePixel;

    PrimRectUVColorCorners(DrawList, p_min, p_max,
        user_texture_id ? uv_min : uv,
        user_texture_id ? uv_max : uv,
        col_upr_left, col_upr_right, col_bot_right, col_bot_left,
        reverse_order);

    if (push_texture_id)
        DrawList->PopTextureID();
}

static void DrawStatic(const ImVec2& center, const ImVec2& resolution, float time)
{
    auto drawList = ImGui::GetBackgroundDrawList();

    const float scaleFactor = Sample(g_noiseScale, time);

    const ImVec2 scale2D = { 1, scaleFactor };

    auto [min, max] = ComputeRect(center, {scale2D.x * resolution.x, scale2D.y * resolution.y});

    // Texture pixel size divided by image resolution (which must be divisible by 4) to get proper UV clipping.
    ImVec2 UV_MAX = { 270.0f / 272.0f,
                      135.0f / 136.0f };

    // Corner points

    ImVec2 topCenter = { center.x, min.y };
    ImVec2 bottomCenter = { center.x, max.y };

    ImVec2 centerLeft = { min.x, center.y };
    ImVec2 centerRight = { max.x, center.y };

    bool flipUV = time >= (73 * FRAME_SCALE);

    const float UV_1 = flipUV ? 0.0f : 1.0f;
    const float UV_0 = flipUV ? 1.0f : 0.0f;

    // Top Left
    AddImageGradient(
        drawList, 
        g_noiseTexture.get(),
        min, 
        center,
        ImVec2(UV_0 * UV_MAX.x, 0.0f * UV_MAX.y),
        ImVec2(0.5f * UV_MAX.x, 0.5f * UV_MAX.y),
        IM_COL32(255, 255, 255, 255 * Sample(g_noiseTL_gradTL, time)),
        IM_COL32(255, 255, 255, 255 * Sample(g_noiseTL_gradTR, time)),
        IM_COL32(255, 255, 255, 255 * Sample(g_noiseTL_gradBR, time)),
        IM_COL32(255, 255, 255, 255 * Sample(g_noiseTL_gradBL, time)),
        true);

    // Top Right
    AddImageGradient(
        drawList, 
        g_noiseTexture.get(),
        topCenter, 
        centerRight,
        ImVec2(0.5f * UV_MAX.x, 0.0f * UV_MAX.y),
        ImVec2(UV_1 * UV_MAX.x, 0.5f * UV_MAX.y),
        IM_COL32(255, 255, 255, 255 * Sample(g_noiseTR_gradTL, time)),
        IM_COL32(255, 255, 255, 255 * Sample(g_noiseTR_gradTR, time)),
        IM_COL32(255, 255, 255, 255 * Sample(g_noiseTR_gradBR, time)),
        IM_COL32(255, 255, 255, 255 * Sample(g_noiseTR_gradBL, time)),
        true);

    // Bottom Right
    AddImageGradient(
        drawList, 
        g_noiseTexture.get(),
        center,
        max,
        ImVec2(0.5f * UV_MAX.x, 0.5f * UV_MAX.y),
        ImVec2(UV_1 * UV_MAX.x, 1.0f * UV_MAX.y),
        IM_COL32(255, 255, 255, 255 * Sample(g_noiseBR_gradTL, time)),
        IM_COL32(255, 255, 255, 255 * Sample(g_noiseBR_gradTR, time)),
        IM_COL32(255, 255, 255, 255 * Sample(g_noiseBR_gradBR, time)),
        IM_COL32(255, 255, 255, 255 * Sample(g_noiseBR_gradBL, time)),
        true);

    // Bottom Left
    AddImageGradient(
        drawList,
        g_noiseTexture.get(),
        centerLeft, 
        bottomCenter,
        ImVec2(UV_0 * UV_MAX.x, 0.5f * UV_MAX.y),
        ImVec2(0.5f * UV_MAX.x, 1.0f * UV_MAX.y),
        IM_COL32(255, 255, 255, 255 * Sample(g_noiseBL_gradTL, time)),
        IM_COL32(255, 255, 255, 255 * Sample(g_noiseBL_gradTR, time)),
        IM_COL32(255, 255, 255, 255 * Sample(g_noiseBL_gradBR, time)),
        IM_COL32(255, 255, 255, 255 * Sample(g_noiseBL_gradBL, time)),
        true);
}

void TVStatic::Init()
{
    g_flashTexture = LOAD_ZSTD_TEXTURE(g_options_static_flash);
    g_noiseTexture = LOAD_ZSTD_TEXTURE(g_options_static);
}

static float ComputeTime(double appearTime)
{
    return (ImGui::GetTime() - appearTime) + FRAME_OFFSET * FRAME_SCALE;
}

float TVStatic::ComputeThumbnailAlpha(double appearTime)
{
    return Sample(g_thumbnailAlpha, ComputeTime(appearTime));
}

void TVStatic::Draw(const ImVec2& center, const ImVec2& resolution, double appearTime)
{
    float time = ComputeTime(appearTime);

    DrawStatic(center, resolution, time);
    DrawFlash(center, resolution, time);
}
