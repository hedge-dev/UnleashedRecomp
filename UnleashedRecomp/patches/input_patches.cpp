#include <api/SWA.h>

static void SetDPadAnalogDirectionX(PPCRegister& pPadState, PPCRegister& x, bool negate)
{
    auto pGuestPadState = (SWA::SPadState*)g_memory.Translate(pPadState.u32);

    if (pGuestPadState->IsDown(SWA::eKeyState_DpadLeft))
        x.f64 = negate ? 1.0f : -1.0f;

    if (pGuestPadState->IsDown(SWA::eKeyState_DpadRight))
        x.f64 = negate ? -1.0f : 1.0f;
}

static void SetDPadAnalogDirectionY(PPCRegister& pPadState, PPCRegister& y, bool negate)
{
    auto pGuestPadState = (SWA::SPadState*)g_memory.Translate(pPadState.u32);

    if (pGuestPadState->IsDown(SWA::eKeyState_DpadUp))
        y.f64 = negate ? -1.0f : 1.0f;

    if (pGuestPadState->IsDown(SWA::eKeyState_DpadDown))
        y.f64 = negate ? 1.0f : -1.0f;
}

void PostureDPadSupportMidAsmHook(PPCRegister& pPadState, PPCRegister& x, PPCRegister& y)
{
    SetDPadAnalogDirectionX(pPadState, x, false);
    SetDPadAnalogDirectionY(pPadState, y, true);
}

void PostureDPadSupportPathLocalMidAsmHook(PPCRegister& pPadState, PPCRegister& x, PPCRegister& y)
{
    SetDPadAnalogDirectionX(pPadState, x, false);
    SetDPadAnalogDirectionY(pPadState, y, false);
}
