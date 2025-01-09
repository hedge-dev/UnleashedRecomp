#include <api/SWA.h>
#include <ui/game_window.h>
#include <user/config.h>
#include "camera_patches.h"

static constexpr float ORIGINAL_ASPECT_RATIO = 4.0f / 3.0f;
static constexpr float ORIGINAL_WIDESCREEN_ASPECT_RATIO = 16.0f / 9.0f;

void CameraAspectRatioMidAsmHook(PPCRegister& r30, PPCRegister& r31)
{
    r30.u32 = 0;

    auto camera = (SWA::CCamera*)g_memory.Translate(r31.u32);

    // Dynamically adjust horizontal aspect ratio to window dimensions.
    camera->m_HorzAspectRatio = float(GameWindow::s_width) / float(GameWindow::s_height);
}

float AdjustFieldOfView(float fieldOfView, float aspectRatio)
{
    if (Config::AspectRatio == EAspectRatio::OriginalNarrow)
    {
        // Replicate the original incorrect field of view formula if requested. 
        fieldOfView *= ORIGINAL_ASPECT_RATIO;
    }
    else if (aspectRatio < ORIGINAL_WIDESCREEN_ASPECT_RATIO)
    {
        // Use proper VERT+ otherwise for narrow aspect ratios.
        fieldOfView = 2.0 * atan(tan(0.5 * fieldOfView) / aspectRatio * ORIGINAL_WIDESCREEN_ASPECT_RATIO);
    }

    return fieldOfView;
}

void CameraFieldOfViewMidAsmHook(PPCRegister& r31, PPCRegister& f31)
{
    auto camera = (SWA::CCamera*)g_memory.Translate(r31.u32);

    f31.f64 = AdjustFieldOfView(f31.f64, camera->m_HorzAspectRatio);
}

PPC_FUNC_IMPL(__imp__sub_824697B0);
PPC_FUNC(sub_824697B0)
{
    auto pCamera = (SWA::CCamera*)g_memory.Translate(ctx.r3.u32);

    pCamera->m_InvertX = Config::InvertCameraX;
    pCamera->m_InvertY = Config::InvertCameraY;

    __imp__sub_824697B0(ctx, base);
}
