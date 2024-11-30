#pragma once

#include <SWA.inl>

namespace Hedgehog::Mirage
{
    class CRenderingInfrastructure;

    class CVertexDeclarationPtr
    {
    public:
        xpointer<void> m_pD3DVertexDeclaration;
        xpointer<CRenderingInfrastructure> m_pRenderingInfrastructure;
    };

    SWA_ASSERT_OFFSETOF(CVertexDeclarationPtr, m_pD3DVertexDeclaration, 0x0);
    SWA_ASSERT_OFFSETOF(CVertexDeclarationPtr, m_pRenderingInfrastructure, 0x4);
    SWA_ASSERT_SIZEOF(CVertexDeclarationPtr, 0x8);
}
