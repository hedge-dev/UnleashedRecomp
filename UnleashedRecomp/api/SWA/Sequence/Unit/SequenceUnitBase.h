#pragma once

#include <SWA.inl>

namespace SWA::Sequence::Unit
{
    class CUnitBase : public Hedgehog::Base::CObject
    {
    public:
        xpointer<void> m_pVftable;
        SWA_INSERT_PADDING(0x14);
    };
}
