#pragma once

#include "CSD/Manager/csdmRCPtr.h"

namespace Chao::CSD
{
    template<typename T>
    class CResourceBase
    {
    public:
        struct Vftable
        {
            be<uint32_t> m_fpDtor;
            be<uint32_t> m_fpCopyResource;
        };

        xpointer<Vftable> m_pVftable;
        RCPtr<uint8_t> m_rcResourceHolder;
        xpointer<T> m_pResource;

        ~CResourceBase();
        void CopyResource(const CResourceBase& in_rOther);
    };
}

#include "CSD/Manager/csdmResourceBase.h"
