#include <cpu/guest_code.h>
#include <user/config.h>
#include <api/SWA.h>
#include <ui/window.h>

// TODO: to be removed.
constexpr float m_baseAspectRatio = 16.0f / 9.0f;

// TODO: to be removed.
void CSDAspectRatioMidAsmHook(PPCRegister& f1, PPCRegister& f2)
{
    if (Config::UIScaleMode == EUIScaleMode::Stretch)
        return;

    auto newAspectRatio = (float)Window::s_width / (float)Window::s_height;

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
    return Config::MotionBlur;
}
