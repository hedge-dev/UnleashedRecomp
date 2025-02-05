#include <kernel/memory.h>
#include <api/SWA.h>
#include <ui/game_window.h>
#include <user/config.h>
#include <app.h>

void DownForceDeltaTimeFixMidAsmHook(PPCRegister& f0)
{
    f0.f64 = 30.0;
}

void DownForceDeltaTimeFixMidAsmHook(PPCVRegister& v127, PPCRegister& f24)
{
    double factor = 1.0 / (f24.f64 * 30.0);
    v127.f32[0] *= factor;
    v127.f32[1] *= factor;
    v127.f32[2] *= factor;
    v127.f32[3] *= factor;
}

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
    t.f64 = ComputeLerpFactor(t.f64, App::s_deltaTime);
}

void CameraTargetSideOffsetLerpFixMidAsmHook(PPCVRegister& v13, PPCVRegister& v62)
{
    float factor = float(ComputeLerpFactor(double(v13.f32[0] * v62.f32[0]), App::s_deltaTime));

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

using namespace std::chrono_literals;

static std::chrono::steady_clock::time_point g_prev;

bool LoadingUpdateMidAsmHook(PPCRegister& r31)
{
    auto now = std::chrono::steady_clock::now();
    double deltaTime = std::min(std::chrono::duration<double>(now - g_prev).count(), 1.0 / 15.0);
    g_prev = now;

    uint8_t* base = g_memory.base;
    uint32_t application = PPC_LOAD_U32(PPC_LOAD_U32(r31.u32 + 4));
    uint32_t update = PPC_LOAD_U32(PPC_LOAD_U32(application) + 20);

    g_ppcContext->r3.u32 = application;
    g_ppcContext->f1.f64 = deltaTime;
    g_memory.FindFunction(update)(*g_ppcContext, base);

    bool loading = *SWA::SGlobals::ms_IsLoading;
    if (loading)
    {
        now = std::chrono::steady_clock::now();
        constexpr auto INTERVAL = 1000000000ns / 30;
        auto next = now + (INTERVAL - now.time_since_epoch() % INTERVAL);

        std::this_thread::sleep_until(next);
    }

    return loading;
}

// ADXM_WaitVsync
PPC_FUNC_IMPL(__imp__sub_8312DBF8);
PPC_FUNC(sub_8312DBF8)
{
    auto now = std::chrono::steady_clock::now();
    constexpr auto INTERVAL = 1000000000ns / 60;
    auto next = now + (INTERVAL - now.time_since_epoch() % INTERVAL);

    std::this_thread::sleep_until(next);
}

void WaitVsyncMidAsmHook()
{
}

void ApplicationFrameLimiterMidAsmHook()
{
}
