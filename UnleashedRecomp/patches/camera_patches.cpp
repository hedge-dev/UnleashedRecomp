#include <api/SWA.h>
#include <ui/game_window.h>
#include <user/config.h>

static constexpr float ORIGINAL_ASPECT_RATIO = 4.0f / 3.0f;
static constexpr float ORIGINAL_WIDESCREEN_ASPECT_RATIO = 16.0f / 9.0f;

void CameraAspectRatioMidAsmHook(PPCRegister& r30, PPCRegister& r31)
{
    r30.u32 = 0;

    auto camera = (SWA::CCamera*)g_memory.Translate(r31.u32);

    // Dynamically adjust horizontal aspect ratio to window dimensions.
    camera->m_HorzAspectRatio = float(GameWindow::s_width) / float(GameWindow::s_height);
}

void CameraFieldOfViewMidAsmHook(PPCRegister& r31, PPCRegister& f31)
{
    auto camera = (SWA::CCamera*)g_memory.Translate(r31.u32);

    // Interpolate to the original 4:3 field of view for narrow aspect ratios.
    if (camera->m_HorzAspectRatio < ORIGINAL_WIDESCREEN_ASPECT_RATIO)
    {
        float factor = std::clamp((camera->m_HorzAspectRatio - ORIGINAL_ASPECT_RATIO) / (ORIGINAL_WIDESCREEN_ASPECT_RATIO - ORIGINAL_ASPECT_RATIO), 0.0f, 1.0f);
        factor = ORIGINAL_ASPECT_RATIO + (1.0f - ORIGINAL_ASPECT_RATIO) * factor;

        f31.f64 *= factor;

        // For tall aspect ratios, use proper VERT+.
        if (camera->m_HorzAspectRatio < ORIGINAL_ASPECT_RATIO)
        {
            f31.f64 = 2.0 * atan(tan(0.5 * f31.f64) / camera->m_HorzAspectRatio * ORIGINAL_ASPECT_RATIO);
        }
    }
}

PPC_FUNC_IMPL(__imp__sub_824697B0);
PPC_FUNC(sub_824697B0)
{
    auto pCamera = (SWA::CCamera*)g_memory.Translate(ctx.r3.u32);

    pCamera->m_InvertX = Config::InvertCameraX;
    pCamera->m_InvertY = Config::InvertCameraY;

    __imp__sub_824697B0(ctx, base);
}
