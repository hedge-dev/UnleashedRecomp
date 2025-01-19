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
        SWA_INSERT_PADDING(0xC4);
        boost::shared_ptr<SWA::CPathController> m_spStageGuidePathController; //xpointer<SWA::CPathController> m_pStageGuidePathController;
        SWA_INSERT_PADDING(0x4);
        Hedgehog::Math::CVector m_PlayerPosition;
        be<float> m_Field0DC; // Sonic's air distance from path??
        SWA_INSERT_PADDING(0x3C);
        be<float> m_StageGuidePathRatio; // Not updated in retail
        be<float> m_StageGuidePathLength;
        SWA_INSERT_PADDING(0x9C);
    };

    SWA_ASSERT_OFFSETOF(CStageManager, m_spStageGuidePathController, 0xC4);
    SWA_ASSERT_OFFSETOF(CStageManager, m_PlayerPosition, 0xD0);
    SWA_ASSERT_OFFSETOF(CStageManager, m_StageGuidePathRatio, 0x11C);
    SWA_ASSERT_OFFSETOF(CStageManager, m_StageGuidePathLength, 0x120);
    SWA_ASSERT_SIZEOF(CStageManager, 0x1C0);
}
