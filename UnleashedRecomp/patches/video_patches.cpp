#include <user/config.h>
#include <api/SWA.h>
#include <ui/game_window.h>
#include <gpu/video.h>
#include "aspect_ratio_patches.h"

using SVertexData = SWA::Sequence::Utility::CPlayMovieWrapper::CRender::SVertexData;

// Update movie player aspect ratio.
PPC_FUNC_IMPL(__imp__sub_82AE30D8);
PPC_FUNC(sub_82AE30D8)
{
    auto pViewportWidth = (be<uint32_t>*)g_memory.Translate(ctx.r4.u32 + 0x14);
    auto pViewportHeight = (be<uint32_t>*)g_memory.Translate(ctx.r4.u32 + 0x18);

    auto pTopLeft = (SVertexData*)g_memory.Translate(ctx.r4.u32 + 0x6C);
    auto pTopRight = (SVertexData*)g_memory.Translate(ctx.r4.u32 + 0x6C + sizeof(SVertexData));
    auto pBottomRight = (SVertexData*)g_memory.Translate(ctx.r4.u32 + 0x6C + sizeof(SVertexData) * 2);
    auto pBottomLeft = (SVertexData*)g_memory.Translate(ctx.r4.u32 + 0x6C + sizeof(SVertexData) * 3);

    auto quadWidth = std::fabs(pTopRight->X - pTopLeft->X) * ((float)*pViewportWidth / 2);
    auto quadHeight = std::fabs(pTopLeft->Y - pBottomLeft->Y) * ((float)*pViewportHeight / 2);

    auto movieAspectRatio = quadWidth / quadHeight;

    auto scaleU = 1.0f;
    auto scaleV = 1.0f;
    auto centreV = (pTopLeft->V + pBottomRight->V) / 2.0f;

    if (g_aspectRatio > movieAspectRatio)
    {
        scaleU = movieAspectRatio / g_aspectRatio;
    }
    else
    {
        scaleV = g_aspectRatio / movieAspectRatio;
    }

    float halfPixelX = 1.0f / Video::s_viewportWidth;
    float halfPixelY = 1.0f / Video::s_viewportHeight;

    pTopLeft->X = -1.0f - halfPixelX;
    pTopLeft->Y = 1.0f + halfPixelY;
    pTopLeft->U = (pTopLeft->U - centreV) / scaleU + centreV;
    pTopLeft->V = (pTopLeft->V - centreV) / scaleV + centreV;

    pTopRight->X = 1.0f - halfPixelX;
    pTopRight->Y = 1.0f + halfPixelY;
    pTopRight->U = (pTopRight->U - centreV) / scaleU + centreV;
    pTopRight->V = (pTopRight->V - centreV) / scaleV + centreV;

    pBottomLeft->X = -1.0f - halfPixelX;
    pBottomLeft->Y = -1.0f + halfPixelY;
    pBottomLeft->U = (pBottomLeft->U - centreV) / scaleU + centreV;
    pBottomLeft->V = (pBottomLeft->V - centreV) / scaleV + centreV;

    pBottomRight->X = 1.0f - halfPixelX;
    pBottomRight->Y = -1.0f + halfPixelY;
    pBottomRight->U = (pBottomRight->U - centreV) / scaleU + centreV;
    pBottomRight->V = (pBottomRight->V - centreV) / scaleV + centreV;

    __imp__sub_82AE30D8(ctx, base);
}

void RemoveMoviePlayerLetterboxMidAsmHook() {}

bool MotionBlurMidAsmHook()
{
    return Config::MotionBlur != EMotionBlur::Off;
}

// Hedgehog::MirageDebug::PrepareRenderPrimitive2D
PPC_FUNC_IMPL(__imp__sub_830D25D8);
PPC_FUNC(sub_830D25D8)
{
    auto device = reinterpret_cast<GuestDevice*>(base + PPC_LOAD_U32(ctx.r4.u32));

    // Set first sampler to use linear filtering.
    device->samplerStates[0].data[3] = (device->samplerStates[0].data[3].get() & ~0x1f80000) | 0x1280000;
    device->dirtyFlags[3] = device->dirtyFlags[3].get() | 0x80000000ull;

    __imp__sub_830D25D8(ctx, base);
}
