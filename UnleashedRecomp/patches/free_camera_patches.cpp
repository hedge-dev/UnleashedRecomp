#include "free_camera_patches.h"
#include <api/SWA.h>
#include <os/logger.h>
#include <ui/game_window.h>
#include <user/config.h>
#include <app.h>

#define DEGREES_TO_RADIANS(x) (float)(x / 180.0f * M_PI)
#define RADIANS_TO_DEGREES(x) (float)(x / M_PI * 180.0f)

constexpr float DEFAULT_SPEED = 1.0f;
constexpr float DEFAULT_FIELD_OF_VIEW = 45.0f;
constexpr float MOVE_SPEED_SLOW = 0.075f;
constexpr float MOVE_SPEED_FAST = 8.0f;
constexpr float MOVE_SPEED_MODIFIER_RATIO = 0.02f;
constexpr float FOV_MODIFIER_RATIO = 1.0f;

static float g_baseSpeed = DEFAULT_SPEED;
static float g_baseFieldOfView = DEFAULT_FIELD_OF_VIEW;

static bool g_isDisablingFreeCamera;
static bool g_isCameraLocked;
static float g_speed;
static float g_fieldOfView;

static void ResetParameters()
{
    g_isCameraLocked = false;
    g_speed = g_baseSpeed = DEFAULT_SPEED;

    *SWA::SGlobals::ms_IsRenderDepthOfField = true;
    FreeCameraPatches::s_fieldOfView = g_fieldOfView = g_baseFieldOfView = DEFAULT_FIELD_OF_VIEW;
}

bool FreeCameraActiveMidAsmHook()
{
    return Config::EnableFreeCamera && FreeCameraPatches::s_isActive;
}

bool FreeCameraSpeedInputMidAsmHook(PPCRegister& r31, PPCRegister& r29, PPCRegister& f0)
{
    if (!Config::EnableFreeCamera)
        return false;

    auto pCamera = (SWA::CFreeCamera*)g_memory.Translate(r31.u32);
    auto pPadState = (SWA::SPadState*)g_memory.Translate(r29.u32);
    auto factor = App::s_deltaTime / (1.0f / 60.0f);

    if (g_isCameraLocked)
    {
        g_speed = 0.0f;
    }
    else
    {
        static auto isLeftTriggerSpeedModifier = false;
        static auto isRightTriggerSpeedModifier = false;

        if (pPadState->IsDown(SWA::eKeyState_LeftTrigger))
        {
            g_speed = MOVE_SPEED_SLOW;
            isLeftTriggerSpeedModifier = true;
        }
        else if (isLeftTriggerSpeedModifier)
        {
            g_speed = g_baseSpeed;
            isLeftTriggerSpeedModifier = false;
        }

        if (pPadState->IsDown(SWA::eKeyState_RightTrigger))
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

        if (pPadState->IsDown(SWA::eKeyState_A))
            g_speed = g_baseSpeed = DEFAULT_SPEED;

        if (pPadState->IsDown(SWA::eKeyState_B))
        {
            g_baseSpeed -= MOVE_SPEED_MODIFIER_RATIO * factor;
            g_speed = g_baseSpeed;

            LOGFN("[Free Camera] Speed: {}", g_speed);
        }

        if (pPadState->IsDown(SWA::eKeyState_X))
        {
            g_baseSpeed += MOVE_SPEED_MODIFIER_RATIO * factor;
            g_speed = g_baseSpeed;

            LOGFN("[Free Camera] Speed: {}", g_speed);
        }

        g_speed = std::clamp(g_speed, 0.01f, 20.0f);
    }

    f0.f64 = g_speed;

    return true;
}

// SWA::CFreeCamera::Update
PPC_FUNC_IMPL(__imp__sub_82472A18);
PPC_FUNC(sub_82472A18)
{
    if (Config::EnableFreeCamera)
    {
        auto pCamera = (SWA::CFreeCamera*)g_memory.Translate(ctx.r3.u32);
        auto aspectRatio = (float)GameWindow::s_width / (float)GameWindow::s_height;

        if (auto pInputState = SWA::CInputState::GetInstance())
        {
            auto& rPadState = pInputState->GetPadState();

            // Deactivate.
            if (rPadState.IsTapped(SWA::eKeyState_Select))
            {
                guest_stack_var<SWA::Message::MsgPopCameraController> msgPopCameraController(pCamera, 0.0f);
                guest_stack_var<SWA::Message::MsgCameraPauseMove> msgCameraPauseMove(false);

                // Process SWA::Message::MsgPopCameraController.
                GuestToHostFunction<int>(sub_8246A840, pCamera->m_pCamera.get(), msgPopCameraController.get());

                // Process SWA::Message::MsgFinishFreeCamera.
                GuestToHostFunction<int>(sub_8253ADB8, App::s_pGameModeStage);

                // Process SWA::Message::MsgCameraPauseMove.
                GuestToHostFunction<int>(sub_824679C0, pCamera->m_pCamera.get(), msgCameraPauseMove.get());

                FreeCameraPatches::s_isActive = false;
                *SWA::SGlobals::ms_IsRenderHud = true;

                LOGN("[Free Camera] Disabled");

                g_isDisablingFreeCamera = true;

                return;
            }

            // Teleport player to camera.
            if (rPadState.IsTapped(SWA::eKeyState_LeftStick))
            {
                guest_stack_var<SWA::Message::MsgSetPosition> msgSetPosition(pCamera->m_Position);
                guest_stack_var<SWA::Message::MsgSetVelocity> msgSetVelocity(Hedgehog::Math::CVector(0.0f, 0.0f, 0.0f));

                if (auto pPlayerSpeedContext = SWA::Player::CPlayerSpeedContext::GetInstance())
                {
                    // Process SWA::Message::MsgSetPosition.
                    GuestToHostFunction<int>(sub_82303100, pPlayerSpeedContext->m_pPlayer.get(), msgSetPosition.get());

                    // Process SWA::Message::MsgSetVelocity.
                    GuestToHostFunction<int>(sub_82311820, pPlayerSpeedContext->m_pPlayer.get(), msgSetVelocity.get());
                }
                else if (App::s_pEvilSonicContext)
                {
                    // Process SWA::Message::MsgSetPosition.
                    GuestToHostFunction<int>(sub_82303100, App::s_pEvilSonicContext->m_pPlayer.get(), msgSetPosition.get());
                }
            }

            // Lock camera.
            if (rPadState.IsTapped(SWA::eKeyState_DpadLeft))
            {
                g_isCameraLocked = !g_isCameraLocked;

                if (g_isCameraLocked)
                {
                    LOGN("[Free Camera] Locked");
                }
                else
                {
                    g_speed = g_baseSpeed;

                    LOGN("[Free Camera] Unlocked");
                }
            }

            // Toggle depth of field.
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

            g_fieldOfView = fmodf(isResetFOV ? DEFAULT_FIELD_OF_VIEW : g_fieldOfView + fovScaleFactor * App::s_deltaTime * 60.0f, 180.0f);

            FreeCameraPatches::s_fieldOfView = 2.0f * atan(tan(DEGREES_TO_RADIANS(g_fieldOfView / 2.0f) * (16.0f / 9.0f / std::min(aspectRatio, 16.0f / 9.0f))));
        }
    }

    __imp__sub_82472A18(ctx, base);
}

void FreeCameraPatches::Update()
{
    if (!Config::EnableFreeCamera || !App::s_pGameModeStage)
        return;

    if (auto pInputState = SWA::CInputState::GetInstance())
    {
        auto& rPadState = pInputState->GetPadState();

        if (rPadState.IsTapped(SWA::eKeyState_Select) && !FreeCameraPatches::s_isActive && !g_isDisablingFreeCamera)
        {
            ResetParameters();

            // Process SWA::Message::MsgStartFreeCamera.
            GuestToHostFunction<int>(sub_8253ACB8, App::s_pGameModeStage);

            FreeCameraPatches::s_isActive = true;
            *SWA::SGlobals::ms_IsRenderHud = false;

            LOGN("[Free Camera] Enabled");
        }

        if (rPadState.IsReleased(SWA::eKeyState_Select) && !FreeCameraPatches::s_isActive)
            g_isDisablingFreeCamera = false;
    }
}
