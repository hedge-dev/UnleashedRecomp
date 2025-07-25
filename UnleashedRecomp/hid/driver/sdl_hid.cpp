#include <stdafx.h>
#include <SDL.h>
#include <user/config.h>
#include <hid/hid.h>
#include <os/logger.h>
#include <ui/game_window.h>
#include <kernel/xdm.h>
#include <app.h>

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
    int index{};

    Controller() = default;

    explicit Controller(int index) : Controller(SDL_GameControllerOpen(index))
    {
        this->index = index;
    }

    Controller(SDL_GameController* controller) : controller(controller)
    {
        if (!controller)
            return;

        joystick = SDL_GameControllerGetJoystick(controller);
        id = SDL_JoystickInstanceID(joystick);
    }

    SDL_GameControllerType GetControllerType() const
    {
        return SDL_GameControllerGetType(controller);
    }

    hid::EInputDevice GetInputDevice() const
    {
        switch (GetControllerType())
        {
            case SDL_CONTROLLER_TYPE_PS3:
            case SDL_CONTROLLER_TYPE_PS4:
            case SDL_CONTROLLER_TYPE_PS5:
                return hid::EInputDevice::PlayStation;
            case SDL_CONTROLLER_TYPE_XBOX360:
            case SDL_CONTROLLER_TYPE_XBOXONE:
                return hid::EInputDevice::Xbox;
            default:
                return hid::EInputDevice::Unknown;
        }
    }

    const char* GetControllerName() const
    {
        auto result = SDL_GameControllerName(controller);

        if (!result)
            return "Unknown Device";

        return result;
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
        return controller;
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
        pad.wButtons |= TRANSLATE_INPUT(SDL_CONTROLLER_BUTTON_TOUCHPAD, XAMINPUT_GAMEPAD_BACK);

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

    void SetLED(const uint8_t r, const uint8_t g, const uint8_t b) const
    {
        SDL_GameControllerSetLED(controller, r, g, b);
    }
};

std::array<Controller, 4> g_controllers;
Controller* g_activeController;

inline Controller* EnsureController(uint32_t dwUserIndex)
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

static void SetControllerInputDevice(Controller* controller)
{
    g_activeController = controller;

    if (App::s_isLoading)
        return;

    hid::g_inputDevice = controller->GetInputDevice();
    hid::g_inputDeviceController = hid::g_inputDevice;

    auto controllerType = (hid::EInputDeviceExplicit)controller->GetControllerType();
    auto controllerName = controller->GetControllerName();

    // Only proceed if the controller type changes.
    if (hid::g_inputDeviceExplicit != controllerType)
    {
        hid::g_inputDeviceExplicit = controllerType;

        if (controllerType == hid::EInputDeviceExplicit::Unknown)
        {
            LOGFN("Detected controller: {} (Unknown Controller Type)", controllerName);
        }
        else
        {
            LOGFN("Detected controller: {}", controllerName);
        }
    }
}

static void SetControllerTimeOfDayLED(Controller& ctrl, bool isNight)
{
    // Fetch the lightbar color based on the time of day
    uint8_t r = 0, g = 0, b = 0;

    if (isNight)
    {
        r = 22;  // Night: Red tone
        g = 0;   // Night: No green
        b = 101; // Night: Cool blue
    }
    else
    {
        r = 0;   // Day: No red
        g = 0;   // Day: No green
        b = 60;  // Day: Dimmer blue
    }

    // Apply the lightbar color to the specified controller
    if (ctrl.CanPoll())
    {
        ctrl.SetLED(r, g, b);
    }
}

int HID_OnSDLEvent(void*, SDL_Event* event)
{
    switch (event->type)
    {
    case SDL_CONTROLLERDEVICEADDED:
    {
        const auto freeIndex = FindFreeController();

        if (freeIndex != -1)
        {
            // Initialize the new controller
            auto controller = Controller(event->cdevice.which);
            g_controllers[freeIndex] = controller;

            // Assign "Player 1 LED" for Player 0 and Player 1
            if (freeIndex == 0 || freeIndex == 1) // Player 0 and Player 1 both use "Player 1 LED"
            {
                SDL_GameControllerSetPlayerIndex(controller.controller, 1); // Force Player LED to "Player 1"
            }
            else // Additional external controllers (Player 2+)
            {
                SDL_GameControllerSetPlayerIndex(controller.controller, freeIndex); // Assign correct LED for Player 2+
            }

            // Apply lightbar settings immediately for the new controller
            SetControllerTimeOfDayLED(controller, App::s_isWerehog);

            // Refresh LEDs for all connected controllers
            for (auto& ctrl : g_controllers)
            {
                if (ctrl.CanPoll())
                {
                    SDL_GameControllerSetPlayerIndex(ctrl.controller, ctrl.index); // Ensure correct LED updates
                }
            }
        }
        break;
    }

    case SDL_CONTROLLERDEVICEREMOVED:
    {
        auto* controller = FindController(event->cdevice.which);

        if (controller)
        {
            controller->Close();

            // If Player 1 disconnects, promote the next available controller to Player 1
            if (controller == &g_controllers[1]) // Player 1 removed
            {
                for (auto& ctrl : g_controllers)
                {
                    if (ctrl.CanPoll() && &ctrl != &g_controllers[0]) // Skip Player 0
                    {
                        g_controllers[1] = ctrl; // Promote next available controller to Player 1
                        SDL_GameControllerSetPlayerIndex(ctrl.controller, 1); // Reflect Player 1 LED
                        SetControllerTimeOfDayLED(ctrl, App::s_isWerehog); // Update lightbar
                        break;
                    }
                }
            }

            // Update Player LED indices for all controllers immediately
            for (std::size_t i = 2; i < g_controllers.size(); ++i) // Start from Player 2 onward
            {
                if (g_controllers[i].CanPoll())
                {
                    SDL_GameControllerSetPlayerIndex(g_controllers[i].controller, static_cast<int>(i)); // Assign correct LED index
                    SetControllerTimeOfDayLED(g_controllers[i], App::s_isWerehog); // Update lightbars
                }
            }

            // Refresh lightbar settings for all connected controllers
            for (auto& ctrl : g_controllers)
            {
                if (ctrl.CanPoll())
                {
                    SetControllerTimeOfDayLED(ctrl, App::s_isWerehog);
                }
            }
        }
        break;
    }

    case SDL_CONTROLLERBUTTONDOWN:
    case SDL_CONTROLLERBUTTONUP:
    case SDL_CONTROLLERAXISMOTION:
    case SDL_CONTROLLERTOUCHPADDOWN:
    {
        auto* controller = FindController(event->cdevice.which);

        if (!controller)
            break;

        // Process input
        if (event->type == SDL_CONTROLLERAXISMOTION)
        {
            if (abs(event->caxis.value) > 8000)
            {
                SDL_ShowCursor(SDL_DISABLE);
                SetControllerInputDevice(controller);
            }

            controller->PollAxis();
        }
        else
        {
            SDL_ShowCursor(SDL_DISABLE);
            SetControllerInputDevice(controller);

            controller->Poll();
        }

        // Instantly apply updated lightbar settings during input
        SetControllerTimeOfDayLED(*controller, App::s_isWerehog);
        break;
    }
    }

    return 0;
}

void hid::Init()
{
    SDL_SetHint(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS, "1");
    SDL_SetHint(SDL_HINT_JOYSTICK_HIDAPI_GAMECUBE, "1");
    SDL_SetHint(SDL_HINT_JOYSTICK_HIDAPI_PS3, "1");
    SDL_SetHint(SDL_HINT_JOYSTICK_HIDAPI_PS4, "1");
    SDL_SetHint(SDL_HINT_JOYSTICK_HIDAPI_PS4_RUMBLE, "1");
    SDL_SetHint(SDL_HINT_JOYSTICK_HIDAPI_PS5, "1");
    SDL_SetHint(SDL_HINT_JOYSTICK_HIDAPI_PS5_PLAYER_LED, "1");
    SDL_SetHint(SDL_HINT_JOYSTICK_HIDAPI_PS5_RUMBLE, "1");
    SDL_SetHint(SDL_HINT_JOYSTICK_HIDAPI_WII, "1");
    SDL_SetHint(SDL_HINT_XINPUT_ENABLED, "1");
    
    SDL_SetHint(SDL_HINT_GAMECONTROLLER_USE_BUTTON_LABELS, "0"); // Uses Button Labels. This hint is disabled for Nintendo Controllers.

    SDL_InitSubSystem(SDL_INIT_EVENTS);
    SDL_AddEventWatch(HID_OnSDLEvent, nullptr);

    SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER);
}

uint32_t hid::GetState(uint32_t dwUserIndex, XAMINPUT_STATE* pState)
{
    static uint32_t packet;

    if (!pState)
        return ERROR_BAD_ARGUMENTS;

    memset(pState, 0, sizeof(*pState));

    pState->dwPacketNumber = packet++;

    if (!g_activeController)
        return ERROR_DEVICE_NOT_CONNECTED;

    pState->Gamepad = g_activeController->state;

    return ERROR_SUCCESS;
}

uint32_t hid::SetState(uint32_t dwUserIndex, XAMINPUT_VIBRATION* pVibration)
{
    if (!pVibration)
        return ERROR_BAD_ARGUMENTS;

    if (!g_activeController)
        return ERROR_DEVICE_NOT_CONNECTED;

    g_activeController->SetVibration(*pVibration);

    return ERROR_SUCCESS;
}

uint32_t hid::GetCapabilities(uint32_t dwUserIndex, XAMINPUT_CAPABILITIES* pCaps)
{
    if (!pCaps)
        return ERROR_BAD_ARGUMENTS;

    if (!g_activeController)
        return ERROR_DEVICE_NOT_CONNECTED;

    memset(pCaps, 0, sizeof(*pCaps));

    pCaps->Type = XAMINPUT_DEVTYPE_GAMEPAD;
    pCaps->SubType = XAMINPUT_DEVSUBTYPE_GAMEPAD; // TODO: other types?
    pCaps->Flags = 0;
    pCaps->Gamepad = g_activeController->state;
    pCaps->Vibration = g_activeController->vibration;

    return ERROR_SUCCESS;
}
