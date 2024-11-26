#include <cpu/guest_code.h>
#include <api/SWA.h>
#include <ui/window.h>
#include <user/config.h>
#include <app.h>

float m_lastLoadingFrameDelta = 0.0f;
std::chrono::steady_clock::time_point m_lastLoadingFrameTime;

void HighFrameRateDeltaTimeFixMidAsmHook(PPCRegister& f1)
{
    // Having 60 FPS threshold ensures we still retain
    // the original game behavior when locked to 30/60 FPS.
    constexpr double threshold = 1.0 / 60.0;

    if (f1.f64 < threshold)
        f1.f64 = threshold;
}

void CameraDeltaTimeFixMidAsmHook(PPCRegister& dest, PPCRegister& src)
{
    dest.f64 = src.f64 / 30.0;
}

void CameraDeltaTimeFixMidAsmHook(PPCRegister& dest)
{
    dest.f64 /= 30.0;
}

static double ComputeLerpFactor(double t, double deltaTime)
{
    // This type of lerp still falls behind when 
    // playing catch with a constantly moving position. 
    // The bias helps with approximately bringing it closer.
    double fps = 1.0 / deltaTime;
    double bias = t * 60.0;
    return 1.0 - pow(1.0 - t, (30.0 + bias) / (fps + bias));
}

// It's important to use global delta time here instead of function provided
// delta time, as it might be time scaled and not match with 30 FPS behavior.
void CameraLerpFixMidAsmHook(PPCRegister& t)
{
    t.f64 = ComputeLerpFactor(t.f64, g_deltaTime);
}

void CameraTargetSideOffsetLerpFixMidAsmHook(PPCVRegister& v13, PPCVRegister& v62)
{
    float factor = float(ComputeLerpFactor(double(v13.f32[0] * v62.f32[0]), g_deltaTime));

    for (size_t i = 0; i < 4; i++)
    {
        v62.f32[i] = factor;
        v13.f32[i] = 1.0f;
    }
}

void Camera2DLerpFixMidAsmHook(PPCRegister& t, PPCRegister& deltaTime)
{
    t.f64 = ComputeLerpFactor(std::min<double>(1.0, t.f64 * 2.0), deltaTime.f64 / 60.0);
}

void Camera2DSlopeLerpFixMidAsmHook(PPCRegister& t, PPCRegister& deltaTime)
{
    t.f64 = ComputeLerpFactor(t.f64, deltaTime.f64 / 60.0);
}

void LoadingScreenSpeedFixMidAsmHook(PPCRegister& r4)
{
    auto now = std::chrono::high_resolution_clock::now();

    m_lastLoadingFrameDelta = std::min(std::chrono::duration<float>(now - m_lastLoadingFrameTime).count(), 1.0f / 15.0f);
    m_lastLoadingFrameTime = now;

    auto pDeltaTime = (be<float>*)g_memory.Translate(r4.u32);

    *pDeltaTime = m_lastLoadingFrameDelta;
}
