#pragma once

#include "CSD/Manager/csdmRCPtr.h"

namespace Chao::CSD
{
    template<typename T>
    class CResourceBase
    {
    public:
        RCPtr<uint8_t> m_rcResourceHolder;
        xpointer<T> m_pResource;

        virtual ~CResourceBase() = default;

        virtual void CopyResource(const CResourceBase& in_rOther);
    };
}
