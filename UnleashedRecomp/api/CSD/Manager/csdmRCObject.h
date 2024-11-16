#pragma once

#include "CSD/Manager/csdmRCPtrAbs.h"

namespace Chao::CSD
{
    class RCPtrAbs::RCObject
    {
    public:
        xpointer<void> m_pMemory;
        be<uint32_t> m_RefCount;
        xpointer<void> m_pDealloctor;
        be<uint32_t> m_eDealloctor;

        virtual ~RCObject();
        virtual void Deallocate(void* in_pMemory) = 0;

        void Release();
    };
}
