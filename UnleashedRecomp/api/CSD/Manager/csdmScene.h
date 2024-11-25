#pragma once

#include "SWA.inl"
#include "CSD/Manager/csdmBase.h"
#include "CSD/Manager/csdmResourceBase.h"
#include "CSD/Manager/csdmSceneObserver.h"
#include "CSD/Manager/csdmSubjectBase.h"

namespace Chao::CSD
{
    struct Scene;
    class CScene;
    class CNode;

    enum EMotionRepeatType : uint32_t
    {
        eMotionRepeatType_PlayOnce = 0,
        eMotionRepeatType_Loop = 1,
        eMotionRepeatType_PingPong = 2,
        eMotionRepeatType_PlayThenDestroy = 3
    };

    class CScene : public CResourceBase<Scene>, SubjectBase<CSceneObserver, CScene>, CBase
    {
    public:
        SWA_INSERT_PADDING(0x60);
        be<float> m_PrevMotionFrame;
        be<float> m_MotionFrame;
        be<float> m_MotionSpeed;
        be<float> m_MotionStartFrame;
        be<float> m_MotionEndFrame;
        SWA_INSERT_PADDING(0x0C);
        be<uint32_t> m_MotionDisableFlag;
        SWA_INSERT_PADDING(0x10);
        be<EMotionRepeatType> m_MotionRepeatType;
        SWA_INSERT_PADDING(0x2C);

        ~CScene();
        void Update(float in_DeltaTime = 0.0f);
        void Render(void* in_pUnk);

        void GetNode(RCPtr<CNode>& out_rResult, const char* in_pName);

        bool SetMotion(const char* in_pName);
        void SetMotionFrame(float in_MotionFrame);
        void SetPosition(float in_X, float in_Y);
        void SetHideFlag(uint32_t in_HideFlag);
        void SetRotation(float in_Angle);
        void SetScale(float in_X, float in_Y);
    };
}

#include "CSD/Manager/csdmScene.inl"
