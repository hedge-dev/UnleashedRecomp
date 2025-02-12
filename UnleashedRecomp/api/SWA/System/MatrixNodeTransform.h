#pragma once

#include <SWA.inl>

namespace SWA
{
    class CMatrixNodeTransform : public Hedgehog::Mirage::CMatrixNode
    {
    public:
        Hedgehog::Mirage::CTransform m_Transform;
        Hedgehog::Math::CMatrix m_WorldMatrix;
    };

    SWA_ASSERT_OFFSETOF(CMatrixNodeTransform, m_Transform, 0x60);
    SWA_ASSERT_OFFSETOF(CMatrixNodeTransform, m_WorldMatrix, 0xC0);
    SWA_ASSERT_SIZEOF(CMatrixNodeTransform, 0x100);
}
