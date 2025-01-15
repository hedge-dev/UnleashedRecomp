#pragma once

#include "SWA.inl"
#include "Hedgehog/Base/Thread/hhSynchronizedObject.h"
#include "Hedgehog/Universe/Engine/hhMessageActor.h"
#include "SWA/System/GameObject.h"


namespace SWA
{
    class CStageManager //: public SWA::CGameObject, public Hedgehog::Base::CSynchronizedObject
    {
    public:
        SWA_INSERT_PADDING(0xD0);
        Hedgehog::Math::CVector m_PlayerPosition; //0xD0
        SWA_INSERT_PADDING(0x40);
        be<float> m_StageGuidePathRatioMaybe; // @ 0x11C
        be<float> m_StageGuidePathLength;    // @ 0x120
        SWA_INSERT_PADDING(0x9C);
    };

    SWA_ASSERT_OFFSETOF(CStageManager, m_PlayerPosition, 0xD0);
    SWA_ASSERT_OFFSETOF(CStageManager, m_StageGuidePathRatioMaybe, 0x11C);
    SWA_ASSERT_OFFSETOF(CStageManager, m_StageGuidePathLength, 0x120);
    SWA_ASSERT_SIZEOF(CStageManager, 0x1C0);
}
