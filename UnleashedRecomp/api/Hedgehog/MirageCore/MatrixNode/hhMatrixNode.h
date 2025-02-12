#pragma once

#include <SWA.inl>

namespace Hedgehog::Mirage
{
    class CMatrixNodeListener;

    class CMatrixNode : public Base::CObject
    {
    public:
        SWA_INSERT_PADDING(0x60);
    };

    SWA_ASSERT_SIZEOF(CMatrixNode, 0x60);
}
