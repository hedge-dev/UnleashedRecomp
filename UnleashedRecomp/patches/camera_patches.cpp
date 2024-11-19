#include <cpu/guest_code.h>
#include <api/SWA.h>
#include <ui/window.h>
#include <cfg/config.h>

constexpr float m_baseAspectRatio = 16.0f / 9.0f;

bool CameraAspectRatioMidAsmHook(PPCRegister& r31)
{
    auto pCamera = (SWA::CCamera*)g_memory.Translate(r31.u32);
    auto newAspectRatio = (float)Window::s_width / (float)Window::s_height;

    // Dynamically adjust horizontal aspect ratio to window dimensions.
    pCamera->m_HorzAspectRatio = newAspectRatio;

    if (auto s_pVertAspectRatio = (be<float>*)g_memory.Translate(0x82028FE0))
    {
        // Dynamically adjust vertical aspect ratio for VERT+.
        *s_pVertAspectRatio = 2.0f * atan(tan(45.0f / 2.0f) * (m_baseAspectRatio / newAspectRatio));
    }

    // Jump to 4:3 code for VERT+ adjustments if using a narrow aspect ratio.
    return newAspectRatio < m_baseAspectRatio;
}

void CameraBoostAspectRatioMidAsmHook(PPCRegister& r31, PPCRegister& f0)
{
    auto pCamera = (SWA::CCamera*)g_memory.Translate(r31.u32);

    if (Window::s_width < Window::s_height)
    {
        // Use horizontal FOV for narrow aspect ratios.
        f0.f32 = pCamera->m_HorzFieldOfView;
    }
    else
    {
        // Use vertical FOV for wide aspect ratios.
        f0.f32 = pCamera->m_VertFieldOfView;
    }
}

PPC_FUNC_IMPL(__imp__sub_824697B0);
PPC_FUNC(sub_824697B0)
{
    auto pCamera = (SWA::CCamera*)g_memory.Translate(ctx.r3.u32);

    pCamera->m_InvertX = Config::CameraXInvert;
    pCamera->m_InvertY = Config::CameraYInvert;

    __imp__sub_824697B0(ctx, base);
}
