#pragma once

#include "CSD/Manager/csdmRCPtr.h"
#include "Hedgehog/Database/System/hhDatabaseData.h"

namespace Chao::CSD
{
    class CProject;
}

namespace SWA
{
    class CCsdProject : public Hedgehog::Database::CDatabaseData
    {
    public:
        Chao::CSD::RCPtr<Chao::CSD::CProject> m_rcProject;
    };
}
