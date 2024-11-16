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
        SWA_INSERT_PADDING(0x5C);
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

        ~CScene() override = default;

        // Update should be called with a delta time of zero
        // after making changes to a motion.

        // Example:
        // SetMotion("Intro_Anim");
        // SetMotionFrame(0.0);
        // m_MotionSpeed = 2.0f;
        // Update(0.0f);

        // Changes are not going to be recognized if
        // update is not called.

        virtual void Update(float in_DeltaTime = 0.0f);
        virtual void Render(void*);

        RCPtr<CNode> GetNode(const char* in_pName) const;

        bool SetMotion(const char* in_pName);
        void SetMotionFrame(float in_MotionFrame);
        void SetPosition(float in_X, float in_Y);
        void SetHideFlag(uint32_t in_HideFlag);
        void SetRotation(float in_Angle);
        void SetScale(float in_X, float in_Y);
    };
}
