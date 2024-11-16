#pragma once

#include "CSD/Platform/csdTexList.h"

namespace Hedgehog::Mirage
{
    class CTexsetData;
}

namespace SWA
{
    class CCsdTexListMirage : public Chao::CSD::CTexList
    {
    public:
        boost::shared_ptr<Hedgehog::Mirage::CTexsetData> m_spTexsetData;
        SWA_INSERT_PADDING(0x04);
    };
}
