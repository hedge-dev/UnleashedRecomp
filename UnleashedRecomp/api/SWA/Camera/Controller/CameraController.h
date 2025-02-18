#pragma once

#include <SWA.inl>

namespace SWA
{
    class CCameraController
    {
    public:
        SWA_INSERT_PADDING(0x08);
        xpointer<CCamera> m_pCamera;
        SWA_INSERT_PADDING(0x58);
        be<float> m_FieldOfView;
        SWA_INSERT_PADDING(0x08);
        Hedgehog::Math::CVector m_Position;
        Hedgehog::Math::CVector m_UpVector;
        Hedgehog::Math::CQuaternion m_Rotation;
        SWA_INSERT_PADDING(0x30);
    };

    SWA_ASSERT_OFFSETOF(CCameraController, m_pCamera, 0x08);
    SWA_ASSERT_OFFSETOF(CCameraController, m_FieldOfView, 0x64);
    SWA_ASSERT_SIZEOF(CCameraController, 0xD0);
}
