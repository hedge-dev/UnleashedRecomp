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
        xpointer<void> m_pVftable;
        Chao::CSD::RCPtr<Chao::CSD::CProject> m_rcProject;
    };
}
