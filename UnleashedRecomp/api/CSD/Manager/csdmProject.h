#pragma once

#include "SWA.inl"
#include "CSD/Manager/csdmBase.h"
#include "CSD/Manager/csdmResourceBase.h"

namespace Chao::CSD
{
    struct Project;
    class CProject;
    class CScene;
    class CTexList;

    class CProject : public CResourceBase<Project>, CBase
    {
    public:
        SWA_INSERT_PADDING(0x1C);
        RCPtr<CTexList> m_rcTexList;
        SWA_INSERT_PADDING(0x1C);

        RCPtr<CScene> CreateScene(const char* in_pName) const;
        RCPtr<CScene> CreateScene(const char* in_pName, const char* in_pMotionName) const;

        void DestroyScene(CScene* in_pScene);
        void DestroyScene(RCPtr<CScene>& inout_rcScene);

        static void DestroyScene(CProject* in_pProject, RCPtr<CScene>& inout_rcScene);
    };
}

#include "CSD/Manager/csdmProject.inl"
