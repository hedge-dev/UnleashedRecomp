#include <api/SWA.h>
#include <hid/hid.h>
#include <ui/sdl_listener.h>
#include <app.h>
#include <exports.h>

class WorldMapTouchParams
{
public:
    float CancelDeadzone{ 0.31f };
    float Damping{ 0.99f };
    float FlickAccelX{ 0.25f };
    float FlickAccelY{ 0.1f };
    float FlickTerminalVelocity{ 40.0f };
    float FlickThreshold{ 2.25f };
    float SensitivityX{};
    float SensitivityY{};
    float Smoothing{ 0.8f };
};

class WorldMapTouchParamsProspero : public WorldMapTouchParams
{
public:
    WorldMapTouchParamsProspero()
    {
        SensitivityX = 1.15f;
        SensitivityY = 1.05f;
    }
}
g_worldMapTouchParamsProspero;

class WorldMapTouchParamsOrbis : public WorldMapTouchParams
{
public:
    WorldMapTouchParamsOrbis()
    {
        SensitivityX = 0.95f;
        SensitivityY = 1.0f;
    }
}
g_worldMapTouchParamsOrbis;

WorldMapTouchParams g_worldMapTouchParams{};

static bool g_isTouchActive;

static float g_worldMapTouchVelocityX;
static float g_worldMapTouchVelocityY;

class SDLEventListenerForInputPatches : public SDLEventListener
{
    static inline int ms_touchpadFingerCount;

    static inline float ms_touchpadX;
    static inline float ms_touchpadY;
    static inline float ms_touchpadDeltaX;
    static inline float ms_touchpadDeltaY;
    static inline float ms_touchpadPrevX;
    static inline float ms_touchpadPrevY;

public:
    static void Update(float deltaTime)
    {
        /* NOTE (Hyper): this code was written at 144Hz and was
           discovered later to be faulty at any other frame rate,
           so this is here to account for that without changing
           all the constants that I had tuned. */
        constexpr auto referenceDeltaTime = 1.0f / 144.0f;

        if (g_isTouchActive)
        {
            auto dxNorm = ms_touchpadDeltaX / referenceDeltaTime;
            auto dyNorm = ms_touchpadDeltaY / referenceDeltaTime;
            auto dxSens = dxNorm * g_worldMapTouchParams.SensitivityX;
            auto dySens = dyNorm * g_worldMapTouchParams.SensitivityY;

            auto smoothing = powf(g_worldMapTouchParams.Smoothing, deltaTime / referenceDeltaTime);

            g_worldMapTouchVelocityX = smoothing * g_worldMapTouchVelocityX + (1.0f - smoothing) * dxSens;
            g_worldMapTouchVelocityY = smoothing * g_worldMapTouchVelocityY + (1.0f - smoothing) * dySens;

            auto flickThreshold = g_worldMapTouchParams.FlickThreshold;

            if (fabs(dxSens) > flickThreshold || fabs(dySens) > flickThreshold)
            {
                g_worldMapTouchVelocityX += dxNorm * g_worldMapTouchParams.FlickAccelX * (deltaTime / referenceDeltaTime);
                g_worldMapTouchVelocityY += dyNorm * g_worldMapTouchParams.FlickAccelY * (deltaTime / referenceDeltaTime);
            }

            auto terminalVelocity = g_worldMapTouchParams.FlickTerminalVelocity;

            g_worldMapTouchVelocityX = std::clamp(g_worldMapTouchVelocityX, -terminalVelocity, terminalVelocity);
            g_worldMapTouchVelocityY = std::clamp(g_worldMapTouchVelocityY, -terminalVelocity, terminalVelocity);
        }
        else
        {
            auto dampingFactor = powf(g_worldMapTouchParams.Damping, deltaTime / referenceDeltaTime);

            g_worldMapTouchVelocityX *= dampingFactor;
            g_worldMapTouchVelocityY *= dampingFactor;
        }
    }

    void OnSDLEvent(SDL_Event* event) override
    {
        switch (event->type)
        {
            case SDL_CONTROLLERTOUCHPADMOTION:
            {
                g_isTouchActive = true;

                if (ms_touchpadFingerCount > 1)
                {
                    g_isTouchActive = false;
                    break;
                }

                ms_touchpadX = event->ctouchpad.x;
                ms_touchpadY = event->ctouchpad.y;
                ms_touchpadDeltaX = ms_touchpadX - ms_touchpadPrevX;
                ms_touchpadDeltaY = ms_touchpadY - ms_touchpadPrevY;
                ms_touchpadPrevX = ms_touchpadX;
                ms_touchpadPrevY = ms_touchpadY;

                break;
            }

            case SDL_CONTROLLERTOUCHPADDOWN:
            {
                g_worldMapTouchParams = hid::g_inputDeviceExplicit == hid::EInputDeviceExplicit::DualSense
                    ? (WorldMapTouchParams)g_worldMapTouchParamsProspero
                    : (WorldMapTouchParams)g_worldMapTouchParamsOrbis;

                ms_touchpadFingerCount++;
                ms_touchpadPrevX = event->ctouchpad.x;
                ms_touchpadPrevY = event->ctouchpad.y;

                break;
            }

            case SDL_CONTROLLERTOUCHPADUP:
                g_isTouchActive = false;
                ms_touchpadFingerCount--;
                break;
        }
    }
}
g_sdlEventListenerForInputPatches;

// -------------- COMMON --------------- //

static bool IsDPadActive(SWA::SPadState* pPadState)
{
    return pPadState->IsDown(SWA::eKeyState_DpadUp)   ||
           pPadState->IsDown(SWA::eKeyState_DpadDown) ||
           pPadState->IsDown(SWA::eKeyState_DpadLeft) ||
           pPadState->IsDown(SWA::eKeyState_DpadRight);
}

static void SetDPadAnalogDirectionX(PPCRegister& pPadState, PPCRegister& x, bool invert, float max = 1.0f)
{
    auto pGuestPadState = (SWA::SPadState*)g_memory.Translate(pPadState.u32);

    if (pGuestPadState->IsDown(SWA::eKeyState_DpadLeft))
        x.f64 = invert ? max : -max;

    if (pGuestPadState->IsDown(SWA::eKeyState_DpadRight))
        x.f64 = invert ? -max : max;
}

static void SetDPadAnalogDirectionY(PPCRegister& pPadState, PPCRegister& y, bool invert, float max = 1.0f)
{
    auto pGuestPadState = (SWA::SPadState*)g_memory.Translate(pPadState.u32);

    if (pGuestPadState->IsDown(SWA::eKeyState_DpadUp))
        y.f64 = invert ? -max : max;

    if (pGuestPadState->IsDown(SWA::eKeyState_DpadDown))
        y.f64 = invert ? max : -max;
}

// -------------- PLAYER --------------- //

void PostureDPadSupportMidAsmHook(PPCRegister& pPadState, PPCRegister& x, PPCRegister& y)
{
    SetDPadAnalogDirectionX(pPadState, x, false);
    SetDPadAnalogDirectionY(pPadState, y, false);
}

void PostureDPadSupportInvertYMidAsmHook(PPCRegister& pPadState, PPCRegister& x, PPCRegister& y)
{
    SetDPadAnalogDirectionX(pPadState, x, false);
    SetDPadAnalogDirectionY(pPadState, y, true);
}

void PostureDPadSupportXMidAsmHook(PPCRegister& pPadState, PPCRegister& x)
{
    SetDPadAnalogDirectionX(pPadState, x, false);
}

void PostureDPadSupportYMidAsmHook(PPCRegister& pPadState, PPCRegister& y)
{
    SetDPadAnalogDirectionY(pPadState, y, false);
}

void PostureSpaceHurrierDPadSupportXMidAsmHook(PPCRegister& pPadState, PPCVRegister& vector)
{
    auto pGuestPadState = (SWA::SPadState*)g_memory.Translate(pPadState.u32);

    if (pGuestPadState->IsDown(SWA::eKeyState_DpadLeft))
        vector.f32[3] = -1.0f;

    if (pGuestPadState->IsDown(SWA::eKeyState_DpadRight))
        vector.f32[3] = 1.0f;
}

void PostureSpaceHurrierDPadSupportYMidAsmHook(PPCRegister& pPadState, PPCVRegister& vector)
{
    auto pGuestPadState = (SWA::SPadState*)g_memory.Translate(pPadState.u32);

    if (pGuestPadState->IsDown(SWA::eKeyState_DpadUp))
        vector.f32[3] = 1.0f;

    if (pGuestPadState->IsDown(SWA::eKeyState_DpadDown))
        vector.f32[3] = -1.0f;
}

// ------------- WORLD MAP ------------- //

bool WorldMapTouchSupportMidAsmHook()
{
    SDLEventListenerForInputPatches::Update(App::s_deltaTime);

    auto vxAbs = fabs(g_worldMapTouchVelocityX);
    auto vyAbs = fabs(g_worldMapTouchVelocityY);

    /* Reduce touch noise if the player has
       their finger resting on the touchpad,
       but allow much precise values without
       touch for proper interpolation to zero. */
    if (vxAbs < 0.05f || vyAbs < 0.05f)
        return !g_isTouchActive;

    return vxAbs > 0 || vyAbs > 0;
}

bool WorldMapTouchMagnetismSupportMidAsmHook(PPCRegister& f0)
{
    return fabs(g_worldMapTouchVelocityX) > f0.f64 || fabs(g_worldMapTouchVelocityY) > f0.f64;
}

void TouchAndDPadSupportWorldMapXMidAsmHook(PPCRegister& pPadState, PPCRegister& x)
{
    auto pGuestPadState = (SWA::SPadState*)g_memory.Translate(pPadState.u32);

    if (fabs(pGuestPadState->LeftStickHorizontal) > g_worldMapTouchParams.CancelDeadzone ||
        fabs(pGuestPadState->LeftStickVertical) > g_worldMapTouchParams.CancelDeadzone)
    {
        g_worldMapTouchVelocityX = 0;
    }

    if (IsDPadActive(pGuestPadState))
    {
        g_worldMapTouchVelocityX = 0;

        SetDPadAnalogDirectionX(pPadState, x, false);
    }
    else
    {
        if (fabs(g_worldMapTouchVelocityX) > 0)
            x.f64 = -g_worldMapTouchVelocityX;
    }
}

void TouchAndDPadSupportWorldMapYMidAsmHook(PPCRegister& pPadState, PPCRegister& y)
{
    auto pGuestPadState = (SWA::SPadState*)g_memory.Translate(pPadState.u32);

    if (fabs(pGuestPadState->LeftStickHorizontal) > g_worldMapTouchParams.CancelDeadzone ||
        fabs(pGuestPadState->LeftStickVertical) > g_worldMapTouchParams.CancelDeadzone)
    {
        g_worldMapTouchVelocityY = 0;
    }

    if (IsDPadActive(pGuestPadState))
    {
        g_worldMapTouchVelocityY = 0;

        SetDPadAnalogDirectionY(pPadState, y, false);
    }
    else
    {
        if (fabs(g_worldMapTouchVelocityY) > 0)
            y.f64 = g_worldMapTouchVelocityY;
    }
}

// SWA::CWorldMapCamera::Update
PPC_FUNC_IMPL(__imp__sub_82486968);
PPC_FUNC(sub_82486968)
{
    auto pWorldMapCamera = (SWA::CWorldMapCamera*)g_memory.Translate(ctx.r3.u32);

    // Reset vertical velocity if maximum pitch reached.
    if (fabs(pWorldMapCamera->m_Pitch) >= 80.0f)
        g_worldMapTouchVelocityY = 0;

    __imp__sub_82486968(ctx, base);
}

// World Map cursor move hook.
PPC_FUNC(sub_8256C938)
{
    auto pWorldMapCursor = (SWA::CWorldMapCursor*)g_memory.Translate(ctx.r3.u32);

    pWorldMapCursor->m_IsCursorMoving = g_isTouchActive;

    if (ctx.r4.u8)
    {
        pWorldMapCursor->m_LeftStickVertical = 0;
        pWorldMapCursor->m_LeftStickHorizontal = 0;
    }
    else if (auto pInputState = SWA::CInputState::GetInstance())
    {
        auto& rPadState = pInputState->GetPadState();

        pWorldMapCursor->m_LeftStickVertical = rPadState.LeftStickVertical;
        pWorldMapCursor->m_LeftStickHorizontal = rPadState.LeftStickHorizontal;

        if (rPadState.IsDown(SWA::eKeyState_DpadUp))
            pWorldMapCursor->m_LeftStickVertical = 1.0f;

        if (rPadState.IsDown(SWA::eKeyState_DpadDown))
            pWorldMapCursor->m_LeftStickVertical = -1.0f;

        if (rPadState.IsDown(SWA::eKeyState_DpadLeft))
            pWorldMapCursor->m_LeftStickHorizontal = -1.0f;

        if (rPadState.IsDown(SWA::eKeyState_DpadRight))
            pWorldMapCursor->m_LeftStickHorizontal = 1.0f;

        if (sqrtf((pWorldMapCursor->m_LeftStickHorizontal * pWorldMapCursor->m_LeftStickHorizontal) +
            (pWorldMapCursor->m_LeftStickVertical * pWorldMapCursor->m_LeftStickVertical)) > 0.7f)
        {
            pWorldMapCursor->m_IsCursorMoving = true;
        }
    }
}
