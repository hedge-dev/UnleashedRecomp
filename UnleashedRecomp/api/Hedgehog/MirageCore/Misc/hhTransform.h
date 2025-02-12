#pragma once

#include <SWA.inl>

namespace Hedgehog::Mirage
{
    class CTransform
    {
    public:
        Math::CQuaternion m_Rotation;
        Math::CVector m_Position;
        Math::CMatrix m_Matrix;
    };

    SWA_ASSERT_OFFSETOF(CTransform, m_Rotation, 0x00);
    SWA_ASSERT_OFFSETOF(CTransform, m_Position, 0x10);
    SWA_ASSERT_OFFSETOF(CTransform, m_Matrix, 0x20);
    SWA_ASSERT_SIZEOF(CTransform, 0x60);
}
