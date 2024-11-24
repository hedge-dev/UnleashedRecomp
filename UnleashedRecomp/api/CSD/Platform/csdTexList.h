#pragma once

#include "CSD/Manager/csdmBase.h"
#include "CSD/Manager/csdmRCPtr.h"

namespace Chao::CSD
{
    class CTexList : public CBase
    {
    public:
        struct Vftable
        {
            be<uint32_t> m_fpDtor;
        };

        xpointer<Vftable> m_pVftable;
        RCPtr<uint8_t> m_rcData;

        ~CTexList();
    };
}

#include "CSD/Platform/csdTexList.inl"
