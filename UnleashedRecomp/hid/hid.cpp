#include "hid.h"

hid::EInputDevice hid::g_inputDevice;
hid::EInputDevice hid::g_inputDeviceController;

uint16_t hid::g_prohibitedButtons;

void hid::SetProhibitedButtons(uint16_t wButtons)
{
    hid::g_prohibitedButtons = wButtons;
}

bool hid::IsInputDeviceController()
{
    return hid::g_inputDevice != hid::EInputDevice::Keyboard &&
        hid::g_inputDevice != hid::EInputDevice::Mouse;
}
