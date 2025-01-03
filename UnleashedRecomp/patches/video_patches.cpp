#include <user/config.h>
#include <api/SWA.h>
#include <ui/game_window.h>

SWA::Sequence::Utility::CPlayMovieWrapper::CRender* g_pPlayMovieWrapperRender;

// TODO: to be removed.
constexpr float m_baseAspectRatio = 16.0f / 9.0f;

// TODO: to be removed.
void CSDAspectRatioMidAsmHook(PPCRegister& f1, PPCRegister& f2)
{
    if (Config::UIScaleMode == EUIScaleMode::Stretch)
        return;

    auto newAspectRatio = (float)GameWindow::s_width / (float)GameWindow::s_height;

    if (newAspectRatio > m_baseAspectRatio)
    {
        f1.f64 = 1280.0f / ((newAspectRatio * 720.0f) / 1280.0f);
    }
    else if (newAspectRatio < m_baseAspectRatio)
    {
        f2.f64 = 720.0f / ((1280.0f / newAspectRatio) / 720.0f);
    }
}

bool MotionBlurMidAsmHook()
{
    return Config::MotionBlur != EMotionBlur::Off;
}

// SWA::Sequence::Utility::CPlayMovieWrapper::CRender
PPC_FUNC_IMPL(__imp__sub_82B732F8);
PPC_FUNC(sub_82B732F8)
{
    g_pPlayMovieWrapperRender = (SWA::Sequence::Utility::CPlayMovieWrapper::CRender*)g_memory.Translate(ctx.r3.u32);

    __imp__sub_82B732F8(ctx, base);
}

// SWA::Sequence::Utility::CPlayMovieWrapper::~CRender
PPC_FUNC_IMPL(__imp__sub_82B71FB0);
PPC_FUNC(sub_82B71FB0)
{
    g_pPlayMovieWrapperRender = nullptr;

    __imp__sub_82B71FB0(ctx, base);
}

// Update movie player dimensions.
PPC_FUNC_IMPL(__imp__sub_82AE2F08);
PPC_FUNC(sub_82AE2F08)
{
    __imp__sub_82AE2F08(ctx, base);

    if (!g_pPlayMovieWrapperRender)
        return;

    // Disable aspect ratio changes so we can do our own.
    g_pPlayMovieWrapperRender->m_MaintainAspectRatio = false;

    auto videoAspectRatio = g_pPlayMovieWrapperRender->m_MovieWidth / g_pPlayMovieWrapperRender->m_MovieHeight;
    auto windowAspectRatio = (float)GameWindow::s_width / (float)GameWindow::s_height;

    float sx, sy, x, y;

    if (windowAspectRatio > videoAspectRatio)
    {
        // Pillarbox.
        sy = 1.0f;
        sx = sy * videoAspectRatio / windowAspectRatio;
        x = (1.0f - sx) / 2.0f;
        y = 0.0f;
    }
    else
    {
        // Letterbox.
        sx = 1.0f;
        sy = sx * windowAspectRatio / videoAspectRatio;
        x = 0.0f;
        y = (1.0f - sy) / 2.0f;
    }

    // TODO: use ImGui element to draw letter/pillarbox over screen edges.
    g_pPlayMovieWrapperRender->m_TopLeftX = -1.0f + x * 2.0f;
    g_pPlayMovieWrapperRender->m_TopLeftY = 1.0f - y * 2.0f;
    g_pPlayMovieWrapperRender->m_TopRightX = -1.0f + (x + sx) * 2.0f;
    g_pPlayMovieWrapperRender->m_TopRightY = 1.0f - y * 2.0f;
    g_pPlayMovieWrapperRender->m_BottomLeftX = -1.0f + x * 2.0f;
    g_pPlayMovieWrapperRender->m_BottomLeftY = 1.0f - (y + sy) * 2.0f;
    g_pPlayMovieWrapperRender->m_BottomRightX = -1.0f + (x + sx) * 2.0f;
    g_pPlayMovieWrapperRender->m_BottomRightY = 1.0f - (y + sy) * 2.0f;
}
