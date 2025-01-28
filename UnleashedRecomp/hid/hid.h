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

    enum class EInputDeviceExplicit
    {
        Unknown,
        Xbox360,
        XboxOne,
        DualShock3,
        DualShock4,
        SwitchPro,
        Virtual,
        DualSense,
        Luna,
        Stadia,
        NvShield,
        SwitchJCLeft,
        SwitchJCRight,
        SwitchJCPair
    };

    extern EInputDevice g_inputDevice;
    extern EInputDevice g_inputDeviceController;
    extern EInputDeviceExplicit g_inputDeviceExplicit;

    extern uint16_t g_prohibitedButtons;

    void Init();

    uint32_t GetState(uint32_t dwUserIndex, XAMINPUT_STATE* pState);
    uint32_t SetState(uint32_t dwUserIndex, XAMINPUT_VIBRATION* pVibration);
    uint32_t GetCapabilities(uint32_t dwUserIndex, XAMINPUT_CAPABILITIES* pCaps);

    void SetProhibitedButtons(uint16_t wButtons);
    bool IsInputAllowed();
    bool IsInputDeviceController();
    std::string GetInputDeviceName();
}
