#pragma once

#include "CSD/Manager/csdmBase.h"
#include "CSD/Manager/csdmRCPtr.h"

namespace Chao::CSD
{
    class CTexList : public CBase
    {
    public:
        RCPtr<uint8_t> m_rcData;

        virtual ~CTexList() = default;
    };
}
