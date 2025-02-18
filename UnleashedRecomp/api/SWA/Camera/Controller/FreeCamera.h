#pragma once

#include <SWA.inl>
#include "SWA/Camera/Controller/CameraController.h"

namespace SWA
{
    class CFreeCamera : public CCameraController
    {
    public:
        SWA_INSERT_PADDING(0x10);
        be<float> m_Speed;
    };

    SWA_ASSERT_OFFSETOF(CFreeCamera, m_Speed, 0xE0);
}
