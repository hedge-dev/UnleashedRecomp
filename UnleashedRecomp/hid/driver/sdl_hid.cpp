#include <stdafx.h>
#include <SDL.h>
#include <user/config.h>
#include <hid/hid_detail.h>
#include <ui/window.h>

#define TRANSLATE_INPUT(S, X) SDL_GameControllerGetButton(controller, S) << FirstBitLow(X)
#define VIBRATION_TIMEOUT_MS 5000

class Controller
{
public:
    SDL_GameController* controller{};
    SDL_Joystick* joystick{};
    SDL_JoystickID id{ -1 };
    XAMINPUT_GAMEPAD state{};
    XAMINPUT_VIBRATION vibration{ 0, 0 };

    Controller() = default;
    explicit Controller(int index) : Controller(SDL_GameControllerOpen(index)) {}

    Controller(SDL_GameController* controller) : controller(controller)
    {
        if (!controller)
            return;

        joystick = SDL_GameControllerGetJoystick(controller);
        id = SDL_JoystickInstanceID(joystick);
    }

    void Close()
    {
        if (!controller)
            return;

        SDL_GameControllerClose(controller);

        controller = nullptr;
        joystick = nullptr;
        id = -1;
    }

    bool CanPoll()
    {
        return controller && (Window::s_isFocused || Config::AllowBackgroundInput);
    }

    void PollAxis()
    {
        if (!CanPoll())
            return;

        auto& pad = state;

        pad.sThumbLX = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTX);
        pad.sThumbLY = ~SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTY);

        pad.sThumbRX = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_RIGHTX);
        pad.sThumbRY = ~SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_RIGHTY);

        pad.bLeftTrigger = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_TRIGGERLEFT) >> 7;
        pad.bRightTrigger = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_TRIGGERRIGHT) >> 7;
    }

    void Poll()
    {
        if (!CanPoll())
            return;

        auto& pad = state;

        pad.wButtons = 0;

        pad.wButtons |= TRANSLATE_INPUT(SDL_CONTROLLER_BUTTON_DPAD_UP, XAMINPUT_GAMEPAD_DPAD_UP);
        pad.wButtons |= TRANSLATE_INPUT(SDL_CONTROLLER_BUTTON_DPAD_DOWN, XAMINPUT_GAMEPAD_DPAD_DOWN);
        pad.wButtons |= TRANSLATE_INPUT(SDL_CONTROLLER_BUTTON_DPAD_LEFT, XAMINPUT_GAMEPAD_DPAD_LEFT);
        pad.wButtons |= TRANSLATE_INPUT(SDL_CONTROLLER_BUTTON_DPAD_RIGHT, XAMINPUT_GAMEPAD_DPAD_RIGHT);

        pad.wButtons |= TRANSLATE_INPUT(SDL_CONTROLLER_BUTTON_START, XAMINPUT_GAMEPAD_START);
        pad.wButtons |= TRANSLATE_INPUT(SDL_CONTROLLER_BUTTON_BACK, XAMINPUT_GAMEPAD_BACK);

        pad.wButtons |= TRANSLATE_INPUT(SDL_CONTROLLER_BUTTON_LEFTSTICK, XAMINPUT_GAMEPAD_LEFT_THUMB);
        pad.wButtons |= TRANSLATE_INPUT(SDL_CONTROLLER_BUTTON_RIGHTSTICK, XAMINPUT_GAMEPAD_RIGHT_THUMB);

        pad.wButtons |= TRANSLATE_INPUT(SDL_CONTROLLER_BUTTON_LEFTSHOULDER, XAMINPUT_GAMEPAD_LEFT_SHOULDER);
        pad.wButtons |= TRANSLATE_INPUT(SDL_CONTROLLER_BUTTON_RIGHTSHOULDER, XAMINPUT_GAMEPAD_RIGHT_SHOULDER);

        pad.wButtons |= TRANSLATE_INPUT(SDL_CONTROLLER_BUTTON_A, XAMINPUT_GAMEPAD_A);
        pad.wButtons |= TRANSLATE_INPUT(SDL_CONTROLLER_BUTTON_B, XAMINPUT_GAMEPAD_B);
        pad.wButtons |= TRANSLATE_INPUT(SDL_CONTROLLER_BUTTON_X, XAMINPUT_GAMEPAD_X);
        pad.wButtons |= TRANSLATE_INPUT(SDL_CONTROLLER_BUTTON_Y, XAMINPUT_GAMEPAD_Y);
    }

    void SetVibration(const XAMINPUT_VIBRATION& vibration)
    {
        if (!CanPoll())
            return;

        this->vibration = vibration;

        SDL_GameControllerRumble(controller, vibration.wLeftMotorSpeed * 256, vibration.wRightMotorSpeed * 256, VIBRATION_TIMEOUT_MS);
    }
};

std::array<Controller, 4> g_controllers;

inline Controller* EnsureController(DWORD dwUserIndex)
{
    if (!g_controllers[dwUserIndex].controller)
        return nullptr;

    return &g_controllers[dwUserIndex];
}

inline size_t FindFreeController()
{
    for (size_t i = 0; i < g_controllers.size(); i++)
    {
        if (!g_controllers[i].controller)
            return i;
    }

    return -1;
}

inline Controller* FindController(int which)
{
    for (auto& controller : g_controllers)
    {
        if (controller.id == which)
            return &controller;
    }

    return nullptr;
}

int HID_OnSDLEvent(void*, SDL_Event* event)
{
    if (event->type >= SDL_CONTROLLERAXISMOTION && event->type < SDL_FINGERDOWN)
    {
        if (event->type == SDL_CONTROLLERDEVICEADDED)
        {
            const auto freeIndex = FindFreeController();

            if (freeIndex != -1)
                g_controllers[freeIndex] = Controller(event->cdevice.which);
        }
        if (event->type == SDL_CONTROLLERDEVICEREMOVED)
        {
            auto* controller = FindController(event->cdevice.which);

            if (controller)
                controller->Close();
        }
        else if (event->type == SDL_CONTROLLERBUTTONDOWN || event->type == SDL_CONTROLLERBUTTONUP || event->type == SDL_CONTROLLERAXISMOTION)
        {
            auto* controller = FindController(event->cdevice.which);

            if (controller)
            {
                if (event->type == SDL_CONTROLLERAXISMOTION)
                {
                    controller->PollAxis();
                }
                else
                {
                    controller->Poll();
                }
            }
        }
    }

    return 0;
}

void hid::detail::Init()
{
    SDL_SetHint(SDL_HINT_JOYSTICK_HIDAPI_PS3, "1");
    SDL_SetHint(SDL_HINT_JOYSTICK_HIDAPI_PS4, "1");
    SDL_SetHint(SDL_HINT_JOYSTICK_HIDAPI_PS5, "1");
    SDL_SetHint(SDL_HINT_JOYSTICK_HIDAPI_WII, "1");
    SDL_SetHint(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS, "1");
    SDL_SetHint(SDL_HINT_XINPUT_ENABLED, "1");

    SDL_InitSubSystem(SDL_INIT_EVENTS);

    SDL_AddEventWatch(HID_OnSDLEvent, nullptr);

    SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER);
}

uint32_t hid::detail::GetState(uint32_t dwUserIndex, XAMINPUT_STATE* pState)
{
    static DWORD packet;

    if (!pState)
        return ERROR_BAD_ARGUMENTS;

    memset(pState, 0, sizeof(*pState));

    pState->dwPacketNumber = packet++;

    SDL_JoystickUpdate();

    if (!EnsureController(dwUserIndex))
        return ERROR_DEVICE_NOT_CONNECTED;

    pState->Gamepad = g_controllers[dwUserIndex].state;

    return ERROR_SUCCESS;
}

uint32_t hid::detail::SetState(uint32_t dwUserIndex, XAMINPUT_VIBRATION* pVibration)
{
    if (!pVibration)
        return ERROR_BAD_ARGUMENTS;

    SDL_JoystickUpdate();

    auto* controller = EnsureController(dwUserIndex);

    if (!controller)
        return ERROR_DEVICE_NOT_CONNECTED;

    controller->SetVibration(*pVibration);

    return ERROR_SUCCESS;
}

uint32_t hid::detail::GetCapabilities(uint32_t dwUserIndex, XAMINPUT_CAPABILITIES* pCaps)
{
    if (!pCaps)
        return ERROR_BAD_ARGUMENTS;

    SDL_JoystickUpdate();

    auto* controller = EnsureController(dwUserIndex);

    if (!controller)
        return ERROR_DEVICE_NOT_CONNECTED;

    memset(pCaps, 0, sizeof(*pCaps));

    pCaps->Type = XAMINPUT_DEVTYPE_GAMEPAD;
    pCaps->SubType = XAMINPUT_DEVSUBTYPE_GAMEPAD; // TODO: other types?
    pCaps->Flags = 0;
    pCaps->Gamepad = controller->state;
    pCaps->Vibration = controller->vibration;

    return ERROR_SUCCESS;
}
