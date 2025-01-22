#pragma once

#include <SWA.inl>

namespace SWA
{
    class CWorldMapCamera : public CCameraController
    {
    public:
        be<float> m_Pitch;
        be<float> m_Yaw;
        be<float> m_Distance;
        be<float> m_RotationSpeed;
        SWA_INSERT_PADDING(0x08);
        bool m_CanMove;
        SWA_INSERT_PADDING(0x34);
        be<float> m_TiltToEarthTransitionSpeed;
    };

    SWA_ASSERT_OFFSETOF(CWorldMapCamera, m_Pitch, 0xD0);
    SWA_ASSERT_OFFSETOF(CWorldMapCamera, m_Yaw, 0xD4);
    SWA_ASSERT_OFFSETOF(CWorldMapCamera, m_Distance, 0xD8);
    SWA_ASSERT_OFFSETOF(CWorldMapCamera, m_RotationSpeed, 0xDC);
    SWA_ASSERT_OFFSETOF(CWorldMapCamera, m_CanMove, 0xE8);
    SWA_ASSERT_OFFSETOF(CWorldMapCamera, m_TiltToEarthTransitionSpeed, 0x120);
}
