#include <api/SWA.h>
#include <hid/hid.h>
#include <ui/sdl_listener.h>
#include <app.h>
#include <exports.h>

constexpr double WORLD_MAP_ROTATE_DEADZONE = 0.69999999;
constexpr double WORLD_MAP_CURSOR_DEADZONE = 0.30000001;

class WorldMapCursorParams
{
public:
    float Damping{ 0.99f };
    float FlickAccelX{ 0.25f };
    float FlickAccelY{ 0.1f };
    float FlickTerminalVelocity{ 40.0f };
    float FlickThreshold{ 2.25f };
    float SensitivityX{};
    float SensitivityY{};
    float Smoothing{ 0.8f };
};

class WorldMapCursorParamsProspero : public WorldMapCursorParams
{
public:
    WorldMapCursorParamsProspero()
    {
        SensitivityX = 1.15f;
        SensitivityY = 1.05f;
    }
}
g_worldMapCursorParamsProspero;

class WorldMapCursorParamsOrbis : public WorldMapCursorParams
{
public:
    WorldMapCursorParamsOrbis()
    {
        SensitivityX = 0.95f;
        SensitivityY = 1.0f;
    }
}
g_worldMapCursorParamsOrbis;

#ifdef UI_KBM_SUPPORT
class WorldMapCursorParamsMouse : public WorldMapCursorParams
{
public:
    WorldMapCursorParamsMouse()
    {
        FlickAccelX = 0.025f;
        FlickAccelY = 0.025f;
        FlickThreshold = 7.5f;
        SensitivityX = 0.15f;
        SensitivityY = 0.15f;
    }
}
g_worldMapCursorParamsMouse;
#endif

WorldMapCursorParams g_worldMapCursorParams{};

static bool g_isCursorActive;

static float g_worldMapCursorVelocityX;
static float g_worldMapCursorVelocityY;

class SDLEventListenerForInputPatches : public SDLEventListener
{
    static inline bool ms_isMouseDown;

    static inline int ms_touchpadFingerCount;

    static inline float ms_cursorX;
    static inline float ms_cursorY;
    static inline float ms_cursorDeltaX;
    static inline float ms_cursorDeltaY;
    static inline float ms_cursorPrevX;
    static inline float ms_cursorPrevY;

public:
    static void Update(float deltaTime)
    {
        /* NOTE (Hyper): this code was written at 144Hz and was
           discovered later to be faulty at any other frame rate,
           so this is here to account for that without changing
           all the constants that I had tuned. */
        constexpr auto referenceDeltaTime = 1.0f / 144.0f;

        if (g_isCursorActive)
        {
            auto dxNorm = ms_cursorDeltaX / referenceDeltaTime;
            auto dyNorm = ms_cursorDeltaY / referenceDeltaTime;
            auto dxSens = dxNorm * g_worldMapCursorParams.SensitivityX;
            auto dySens = dyNorm * g_worldMapCursorParams.SensitivityY;

            auto smoothing = powf(g_worldMapCursorParams.Smoothing, deltaTime / referenceDeltaTime);

            g_worldMapCursorVelocityX = smoothing * g_worldMapCursorVelocityX + (1.0f - smoothing) * dxSens;
            g_worldMapCursorVelocityY = smoothing * g_worldMapCursorVelocityY + (1.0f - smoothing) * dySens;

            auto flickThreshold = g_worldMapCursorParams.FlickThreshold;

            if (fabs(dxSens) > flickThreshold || fabs(dySens) > flickThreshold)
            {
                g_worldMapCursorVelocityX += dxNorm * g_worldMapCursorParams.FlickAccelX * (deltaTime / referenceDeltaTime);
                g_worldMapCursorVelocityY += dyNorm * g_worldMapCursorParams.FlickAccelY * (deltaTime / referenceDeltaTime);
            }

            auto terminalVelocity = g_worldMapCursorParams.FlickTerminalVelocity;

            g_worldMapCursorVelocityX = std::clamp(g_worldMapCursorVelocityX, -terminalVelocity, terminalVelocity);
            g_worldMapCursorVelocityY = std::clamp(g_worldMapCursorVelocityY, -terminalVelocity, terminalVelocity);
        }
        else
        {
            auto dampingFactor = powf(g_worldMapCursorParams.Damping, deltaTime / referenceDeltaTime);

            g_worldMapCursorVelocityX *= dampingFactor;
            g_worldMapCursorVelocityY *= dampingFactor;
        }
    }

    void OnSDLEvent(SDL_Event* event) override
    {
        if (!hid::IsInputAllowed())
            return;

        switch (event->type)
        {
#ifdef UI_KBM_SUPPORT
            case SDL_MOUSEMOTION:
            {
                if (!ms_isMouseDown)
                    break;
            
                g_isCursorActive = true;
            
                ms_cursorDeltaX = (float)event->motion.xrel / 100.0f;
                ms_cursorDeltaY = (float)event->motion.yrel / 100.0f;
            
                break;
            }
            
            case SDL_MOUSEBUTTONDOWN:
            {
                if (event->button.button == SDL_BUTTON_LEFT)
                {
                    g_worldMapCursorParams = g_worldMapCursorParamsMouse;
                    ms_isMouseDown = true;
                }
            
                break;
            }
            
            case SDL_MOUSEBUTTONUP:
            {
                if (event->button.button == SDL_BUTTON_LEFT)
                {
                    g_isCursorActive = false;
                    ms_isMouseDown = false;
                }
            
                break;
            }
#endif
            case SDL_CONTROLLERTOUCHPADMOTION:
            {
                g_isCursorActive = true;

                if (ms_touchpadFingerCount > 1)
                {
                    g_isCursorActive = false;
                    break;
                }

                ms_cursorX = event->ctouchpad.x;
                ms_cursorY = event->ctouchpad.y;
                ms_cursorDeltaX = ms_cursorX - ms_cursorPrevX;
                ms_cursorDeltaY = ms_cursorY - ms_cursorPrevY;
                ms_cursorPrevX = ms_cursorX;
                ms_cursorPrevY = ms_cursorY;

                break;
            }

            case SDL_CONTROLLERTOUCHPADDOWN:
            {
                g_worldMapCursorParams = hid::g_inputDeviceExplicit == hid::EInputDeviceExplicit::DualSense
                    ? (WorldMapCursorParams)g_worldMapCursorParamsProspero
                    : (WorldMapCursorParams)g_worldMapCursorParamsOrbis;

                ms_touchpadFingerCount++;
                ms_cursorPrevX = event->ctouchpad.x;
                ms_cursorPrevY = event->ctouchpad.y;

                break;
            }

            case SDL_CONTROLLERTOUCHPADUP:
                g_isCursorActive = false;
                ms_touchpadFingerCount--;
                break;
        }
    }
}
g_sdlEventListenerForInputPatches;

// -------------- COMMON --------------- //

static bool IsDPadThreshold(const SWA::SPadState* pPadState)
{
    return pPadState->IsDown(SWA::eKeyState_DpadUp)   ||
           pPadState->IsDown(SWA::eKeyState_DpadDown) ||
           pPadState->IsDown(SWA::eKeyState_DpadLeft) ||
           pPadState->IsDown(SWA::eKeyState_DpadRight);
}

static bool IsLeftStickThreshold(const SWA::SPadState* pPadState, double deadzone = 0)
{
    return sqrtl((pPadState->LeftStickHorizontal * pPadState->LeftStickHorizontal) +
        (pPadState->LeftStickVertical * pPadState->LeftStickVertical)) > deadzone;
}

static bool IsCursorThreshold(double deadzone = 0, bool isBelowThreshold = false)
{
    auto sqrt = sqrtl((g_worldMapCursorVelocityX * g_worldMapCursorVelocityX) +
        (g_worldMapCursorVelocityY * g_worldMapCursorVelocityY));

    if (isBelowThreshold)
        return sqrt < deadzone;

    return sqrt >= deadzone;
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

bool WorldMapDeadzoneMidAsmHook(PPCRegister& pPadState)
{
    auto pGuestPadState = (SWA::SPadState*)g_memory.Translate(pPadState.u32);

    if (IsDPadThreshold(pGuestPadState) || IsLeftStickThreshold(pGuestPadState))
    {
        g_worldMapCursorVelocityX = 0;
        g_worldMapCursorVelocityY = 0;
    }
    else
    {
        SDLEventListenerForInputPatches::Update(App::s_deltaTime);

        /* Reduce noise if the cursor is resting in
           place, but allow much precise values for
           proper interpolation to zero. */
        if (IsCursorThreshold(0.05, true))
            return !g_isCursorActive;

        return IsCursorThreshold();
    }

    return IsDPadThreshold(pGuestPadState) || IsLeftStickThreshold(pGuestPadState, WORLD_MAP_ROTATE_DEADZONE);
}

bool WorldMapMagnetismMidAsmHook(PPCRegister& f0)
{
    if (IsCursorThreshold(f0.f64, true))
    {
        g_worldMapCursorVelocityX = 0;
        g_worldMapCursorVelocityY = 0;

        return true;
    }

    return false;
}

void WorldMapHidSupportXMidAsmHook(PPCRegister& pPadState, PPCRegister& x)
{
    auto pGuestPadState = (SWA::SPadState*)g_memory.Translate(pPadState.u32);

    if (IsDPadThreshold(pGuestPadState))
    {
        SetDPadAnalogDirectionX(pPadState, x, false);
    }
    else if (fabs(g_worldMapCursorVelocityX) > 0)
    {
        x.f64 = -g_worldMapCursorVelocityX;
    }
}

void WorldMapHidSupportYMidAsmHook(PPCRegister& pPadState, PPCRegister& y)
{
    auto pGuestPadState = (SWA::SPadState*)g_memory.Translate(pPadState.u32);

    if (IsDPadThreshold(pGuestPadState))
    {
        SetDPadAnalogDirectionY(pPadState, y, false);
    }
    else if (fabs(g_worldMapCursorVelocityY) > 0)
    {
        y.f64 = g_worldMapCursorVelocityY;
    }
}

// SWA::CWorldMapCamera::Update
PPC_FUNC_IMPL(__imp__sub_82486968);
PPC_FUNC(sub_82486968)
{
    auto pWorldMapCamera = (SWA::CWorldMapCamera*)g_memory.Translate(ctx.r3.u32);

    // Reset vertical velocity if maximum pitch reached.
    if (fabs(pWorldMapCamera->m_Pitch) >= 80.0f)
        g_worldMapCursorVelocityY = 0;

    __imp__sub_82486968(ctx, base);
}

// World Map cursor move hook.
PPC_FUNC(sub_8256C938)
{
    auto pWorldMapCursor = (SWA::CWorldMapCursor*)g_memory.Translate(ctx.r3.u32);

    pWorldMapCursor->m_IsCursorMoving = g_isCursorActive && IsCursorThreshold(1.0);

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

        if (sqrtl((pWorldMapCursor->m_LeftStickHorizontal * pWorldMapCursor->m_LeftStickHorizontal) +
            (pWorldMapCursor->m_LeftStickVertical * pWorldMapCursor->m_LeftStickVertical)) > WORLD_MAP_ROTATE_DEADZONE)
        {
            pWorldMapCursor->m_IsCursorMoving = true;
        }
    }
}
