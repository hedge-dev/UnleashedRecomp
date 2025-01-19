#pragma once

namespace hid
{
    enum class EInputDevice
    {
        Keyboard,
        Mouse,
        Xbox,
        PlayStation
    };

    extern EInputDevice g_inputDevice;
    extern EInputDevice g_inputDeviceController;

    extern uint16_t g_prohibitedButtons;

    void Init();
    void SetProhibitedButtons(uint16_t wButtons);

    uint32_t GetState(uint32_t dwUserIndex, XAMINPUT_STATE* pState);
    uint32_t SetState(uint32_t dwUserIndex, XAMINPUT_VIBRATION* pVibration);
    uint32_t GetCapabilities(uint32_t dwUserIndex, XAMINPUT_CAPABILITIES* pCaps);

    bool IsInputDeviceController();
}
