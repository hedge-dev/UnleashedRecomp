#include <cpu/guest_code.h>
#include <api/SWA.h>
#include <ui/game_window.h>
#include <user/config.h>

constexpr float m_baseAspectRatio = 16.0f / 9.0f;

bool CameraAspectRatioMidAsmHook(PPCRegister& r31)
{
    auto pCamera = (SWA::CCamera*)g_memory.Translate(r31.u32);
    auto newAspectRatio = (float)GameWindow::s_width / (float)GameWindow::s_height;

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

bool CameraBoostAspectRatioMidAsmHook(PPCRegister& r31, PPCRegister& f0, PPCRegister& f10, PPCRegister& f12)
{
    auto pCamera = (SWA::CCamera*)g_memory.Translate(r31.u32);

    if (GameWindow::s_width < GameWindow::s_height)
    {
        pCamera->m_VertFieldOfView = pCamera->m_HorzFieldOfView + f10.f64;
    }
    else
    {
        pCamera->m_VertFieldOfView = (f12.f64 / f0.f64) + f10.f64;
    }

    return true;
}

PPC_FUNC_IMPL(__imp__sub_824697B0);
PPC_FUNC(sub_824697B0)
{
    auto pCamera = (SWA::CCamera*)g_memory.Translate(ctx.r3.u32);

    pCamera->m_InvertX = Config::InvertCameraX;
    pCamera->m_InvertY = Config::InvertCameraY;

    __imp__sub_824697B0(ctx, base);
}
