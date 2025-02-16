#pragma once

#include <SWA.inl>
#include "SWA/Camera/CameraController.h"

namespace SWA
{
    class CReplayFreeCamera : public CCameraController
    {
    public:
        SWA_INSERT_PADDING(0x90);
        be<float> m_Speed;
    };

    SWA_ASSERT_OFFSETOF(CReplayFreeCamera, m_Speed, 0x160);
}
