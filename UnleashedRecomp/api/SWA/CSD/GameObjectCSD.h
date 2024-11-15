#pragma once

#include "SWA.inl"
#include "CSD/Manager/csdmRCPtr.h"
#include "SWA/System/GameObject.h"

namespace Chao::CSD
{
    class CProject;
}

namespace SWA
{
    class CGameObjectCSD : public CGameObject
    {
    public:
        Chao::CSD::RCPtr<Chao::CSD::CProject> m_rcProject;
        SWA_INSERT_PADDING(0x38);
    };
}
