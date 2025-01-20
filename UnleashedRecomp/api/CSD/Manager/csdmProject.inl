#include "kernel/function.h"

namespace Chao::CSD
{
    inline RCPtr<CScene> CProject::CreateScene(const char* in_pName) const
    {
        RCPtr<CScene> rcScene;
        GuestToHostFunction<void>(sub_830BEE00, this, rcScene, in_pName, nullptr);
        return rcScene;
    }

    inline RCPtr<CScene> CProject::CreateScene(const char* in_pName, const char* in_pMotionName) const
    {
        RCPtr<CScene> rcScene;
        GuestToHostFunction<void>(sub_830BECE0, this, rcScene, in_pName, in_pMotionName, nullptr);
        return rcScene;
    }

    inline void CProject::DestroyScene(CScene* in_pScene)
    {
        GuestToHostFunction<void>(sub_830BE298, this, in_pScene);
    }

    inline void CProject::DestroyScene(RCPtr<CScene>& inout_rcScene)
    {
        if (!inout_rcScene)
            return;

        DestroyScene(inout_rcScene.Get());
        inout_rcScene = nullptr;
    }

    inline void CProject::DestroyScene(CProject* in_pProject, RCPtr<CScene>& inout_rcScene)
    {
        if (in_pProject)
            in_pProject->DestroyScene(inout_rcScene);
        else
            inout_rcScene = nullptr;
    }
}
