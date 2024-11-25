#pragma once

#include "CSD/Manager/csdmRCPtrAbs.h"

namespace Chao::CSD
{
    class RCPtrAbs::RCObject
    {
    public:
        struct Vftable
        {
            be<uint32_t> m_fpDtor;
            be<uint32_t> m_fpDeallocate;
        };

        xpointer<Vftable> m_pVftable;
        xpointer<void> m_pMemory;
        be<uint32_t> m_RefCount;
        xpointer<void> m_pDealloctor;
        be<uint32_t> m_eDealloctor;

        ~RCObject();
        void Deallocate(void* in_pMemory);
        void Release();
    };
}

#include "CSD/Manager/csdmRCObject.inl"
