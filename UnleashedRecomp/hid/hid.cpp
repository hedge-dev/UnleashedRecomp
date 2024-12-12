#include <stdafx.h>
#include "hid.h"
#include "hid_detail.h"

hid::detail::EInputDevice hid::detail::g_inputDevice;
hid::detail::EInputDevice hid::detail::g_inputDeviceController;

void hid::Init()
{
    detail::Init();
}

uint32_t hid::GetState(uint32_t dwUserIndex, XAMINPUT_STATE* pState)
{
    return detail::GetState(dwUserIndex, pState);
}

uint32_t hid::SetState(uint32_t dwUserIndex, XAMINPUT_VIBRATION* pVibration)
{
    return detail::SetState(dwUserIndex, pVibration);
}

uint32_t hid::GetCapabilities(uint32_t dwUserIndex, XAMINPUT_CAPABILITIES* pCaps)
{
    return detail::GetCapabilities(dwUserIndex, pCaps);
}

bool hid::detail::IsInputDeviceController()
{
    return hid::detail::g_inputDevice != hid::detail::EInputDevice::Keyboard &&
        hid::detail::g_inputDevice != hid::detail::EInputDevice::Mouse;
}
