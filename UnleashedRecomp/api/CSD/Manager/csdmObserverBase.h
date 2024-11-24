#pragma once

#include "SWA.inl"

namespace Chao::CSD
{
    template<typename TObservee>
    class CObserverBase
    {
    public:
        struct Vftable
        {
            be<uint32_t> m_fpDtor;
        };

        xpointer<Vftable> m_pVftable;
        SWA_INSERT_PADDING(0x08);

        ~CObserverBase();
    };
}

#include "CSD/Manager/csdmObserverBase.inl"
