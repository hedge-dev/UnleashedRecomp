#include "free_camera_patches.h"
#include <api/SWA.h>
#include <os/logger.h>
#include <ui/game_window.h>
#include <user/config.h>
#include <app.h>

#define DEGREES_TO_RADIANS(x) (float)(x / 180.0f * M_PI)
#define RADIANS_TO_DEGREES(x) (float)(x / M_PI * 180.0f)

constexpr float DEFAULT_SPEED = 0.5f;
constexpr float DEFAULT_FOV = 45.0f;
constexpr float MOVE_SPEED_SLOW = 0.075f;
constexpr float MOVE_SPEED_FAST = 8.0f;
constexpr float MOVE_SPEED_MODIFIER_RATIO = 0.02f;
constexpr float FOV_MODIFIER_RATIO = 1.0f;

static float g_baseSpeed = DEFAULT_SPEED;
static float g_baseFOV = DEFAULT_FOV;

static bool g_isCameraLocked;
static float g_speed;
static float g_fov;

static void ResetParameters()
{
    g_isCameraLocked = false;
    g_speed = g_baseSpeed = DEFAULT_SPEED;

    *SWA::SGlobals::ms_IsRenderDepthOfField = true;
    FreeCameraPatches::s_fieldOfView = g_fov = g_baseFOV = DEFAULT_FOV;
}

bool EnableFreeCameraMidAsmHook()
{
    return Config::EnableFreeCamera;
}

bool FreeCameraNullInputMidAsmHook()
{
    return Config::EnableFreeCamera;
}

// Original input: D-Pad Up
bool FreeCameraActivationInputMidAsmHook(PPCRegister& r11, PPCRegister& r27, PPCRegister& r28)
{
    if (!Config::EnableFreeCamera)
        return false;

    static auto isChangedCameraMode = false;

    if ((r11.u32 & SWA::eKeyState_Select) != 0)
    {
        if (++r28.u32 >= 2)
            r28.u32 = 0;

        isChangedCameraMode = true;
    }

    FreeCameraPatches::s_isActive = r28.u32 > 0;

    if (isChangedCameraMode)
    {
        ResetParameters();

        switch (r28.u32)
        {
            case 0:
                LOGN("[Free Camera] Disabled");
                break;

            case 1:
                LOGN("[Free Camera] Enabled");
                break;
        }

        isChangedCameraMode = false;

        if (FreeCameraPatches::s_isActive && *SWA::SGlobals::ms_IsRenderHud)
        {
            *SWA::SGlobals::ms_IsRenderHud = false;
        }
        else
        {
            *SWA::SGlobals::ms_IsRenderHud = true;
        }
    }

    return true;
}

// Original input: D-Pad Left
void FreeCameraTeleportToPlayerInputMidAsmHook(PPCRegister& r4)
{
    if (Config::EnableFreeCamera)
        r4.u32 = SWA::eKeyState_RightStick;
}

// Original inputs: X (Square) / Y (Triangle)
bool FreeCameraSpeedInputMidAsmHook(PPCRegister& r31)
{
    if (!Config::EnableFreeCamera)
        return false;

    auto pCamera = (SWA::CReplayFreeCamera*)g_memory.Translate(r31.u32);
    auto pInputState = SWA::CInputState::GetInstance();
    
    if (!pInputState)
        return false;

    auto& rPadState = pInputState->GetPadState();

    auto factor = App::s_deltaTime / (1.0f / 60.0f);
    auto aspectRatio = (float)GameWindow::s_width / (float)GameWindow::s_height;

    if (g_isCameraLocked)
    {
        g_speed = 0.0f;
    }
    else
    {
        static auto isLeftTriggerSpeedModifier = false;
        static auto isRightTriggerSpeedModifier = false;

        if (rPadState.IsDown(SWA::eKeyState_LeftTrigger))
        {
            g_speed = MOVE_SPEED_SLOW;
            isLeftTriggerSpeedModifier = true;
        }
        else if (isLeftTriggerSpeedModifier)
        {
            g_speed = g_baseSpeed;
            isLeftTriggerSpeedModifier = false;
        }

        if (rPadState.IsDown(SWA::eKeyState_RightTrigger))
        {
            g_speed = MOVE_SPEED_FAST;
            isRightTriggerSpeedModifier = true;
        }
        else if (isRightTriggerSpeedModifier)
        {
            g_speed = g_baseSpeed;
            isRightTriggerSpeedModifier = false;
        }

        if (isLeftTriggerSpeedModifier && isRightTriggerSpeedModifier)
            g_speed = MOVE_SPEED_FAST / 3;

        if (rPadState.IsDown(SWA::eKeyState_A))
            g_speed = g_baseSpeed = DEFAULT_SPEED;

        if (rPadState.IsDown(SWA::eKeyState_B))
        {
            g_baseSpeed -= MOVE_SPEED_MODIFIER_RATIO * factor;
            g_speed = g_baseSpeed;

            LOGFN("[Free Camera] Speed: {}", g_speed);
        }

        if (rPadState.IsDown(SWA::eKeyState_X))
        {
            g_baseSpeed += MOVE_SPEED_MODIFIER_RATIO * factor;
            g_speed = g_baseSpeed;

            LOGFN("[Free Camera] Speed: {}", g_speed);
        }

        auto isResetFOV = rPadState.IsDown(SWA::eKeyState_Y);
        auto isIncreaseFOV = rPadState.IsDown(SWA::eKeyState_DpadUp);
        auto isDecreaseFOV = rPadState.IsDown(SWA::eKeyState_DpadDown);

        auto fovScaleFactor = 0.0f;

        if (isIncreaseFOV)
        {
            fovScaleFactor = FOV_MODIFIER_RATIO;
        }
        else if (isDecreaseFOV)
        {
            fovScaleFactor = -FOV_MODIFIER_RATIO;
        }

        g_speed = std::clamp(g_speed, 0.01f, 20.0f);
        g_fov = fmodf(isResetFOV ? DEFAULT_FOV : g_fov + fovScaleFactor * App::s_deltaTime * 60.0f, 180.0f);
    }

    if (rPadState.IsTapped(SWA::eKeyState_DpadLeft))
    {
        g_isCameraLocked = !g_isCameraLocked;
        g_speed = g_baseSpeed;

        if (g_isCameraLocked)
        {
            LOGN("[Free Camera] Locked");
        }
        else
        {
            LOGN("[Free Camera] Unlocked");
        }
    }

    if (rPadState.IsTapped(SWA::eKeyState_DpadRight))
    {
        *SWA::SGlobals::ms_IsRenderDepthOfField = !*SWA::SGlobals::ms_IsRenderDepthOfField;

        if (*SWA::SGlobals::ms_IsRenderDepthOfField)
        {
            LOGN("[Free Camera] Depth of Field ON");
        }
        else
        {
            LOGN("[Free Camera] Depth of Field OFF");
        }
    }

    pCamera->m_Speed = g_speed;

    FreeCameraPatches::s_fieldOfView = 2.0f * atan(tan(DEGREES_TO_RADIANS(g_fov / 2.0f) * (16.0f / 9.0f / std::min(aspectRatio, 16.0f / 9.0f))));

    return true;
}
