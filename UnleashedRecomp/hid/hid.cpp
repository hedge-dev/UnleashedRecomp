#include "hid.h"
#include <ui/game_window.h>
#include <user/config.h>

hid::EInputDevice hid::g_inputDevice;
hid::EInputDevice hid::g_inputDeviceController;
hid::EInputDeviceExplicit hid::g_inputDeviceExplicit;

uint16_t hid::g_prohibitedButtons;

void hid::SetProhibitedButtons(uint16_t wButtons)
{
    hid::g_prohibitedButtons = wButtons;
}

bool hid::IsInputAllowed()
{
    return GameWindow::s_isFocused || Config::AllowBackgroundInput;
}

bool hid::IsInputDeviceController()
{
    return hid::g_inputDevice != hid::EInputDevice::Keyboard &&
        hid::g_inputDevice != hid::EInputDevice::Mouse;
}

std::string hid::GetInputDeviceName()
{
    switch (g_inputDevice)
    {
        case EInputDevice::Keyboard:
            return "Keyboard";

        case EInputDevice::Mouse:
            return "Mouse";
    }

    switch (g_inputDeviceExplicit)
    {
        case EInputDeviceExplicit::Xbox360:
            return "Xbox 360";

        case EInputDeviceExplicit::XboxOne:
            return "Xbox One";

        case EInputDeviceExplicit::DualShock3:
            return "DualShock 3";

        case EInputDeviceExplicit::DualShock4:
            return "DualShock 4";

        case EInputDeviceExplicit::SwitchPro:
            return "Nintendo Switch Pro";

        case EInputDeviceExplicit::Virtual:
            return "Virtual";

        case EInputDeviceExplicit::DualSense:
            return "DualSense";

        case EInputDeviceExplicit::Luna:
            return "Amazon Luna";

        case EInputDeviceExplicit::Stadia:
            return "Google Stadia";

        case EInputDeviceExplicit::NvShield:
            return "NVIDIA Shield";

        case EInputDeviceExplicit::SwitchJCLeft:
            return "Nintendo Switch Joy-Con (Left)";

        case EInputDeviceExplicit::SwitchJCRight:
            return "Nintendo Switch Joy-Con (Right)";

        case EInputDeviceExplicit::SwitchJCPair:
            return "Nintendo Switch Joy-Con (Pair)";
    }

    return "Unknown";
}
