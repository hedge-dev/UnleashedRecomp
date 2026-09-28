#pragma once

namespace TouchHaptics
{
    void Init();

    // Plays a short tap on the Taptic Engine. Light is used for the stick, medium for buttons.
    void Play(bool light);
}
