#pragma once

#include <xbox.h>

namespace TouchControls
{
    void Init();
    void Draw();
    bool IsActive();
    bool IsEnabled();
    XAMINPUT_GAMEPAD GetState();
}
