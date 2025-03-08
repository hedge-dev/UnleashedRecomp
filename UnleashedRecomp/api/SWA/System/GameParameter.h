#pragma once

#include <SWA.inl>

namespace SWA
{
    class CGameParameter // : public Hedgehog::Universe::CMessageActor
    {
    public:
        struct SSaveData;
        struct SStageParameter 
        {
            hh::map<int, int> Field00;
            xpointer<void> FieldC;
            xpointer<void> Field10;
            xpointer<void> Field14;
            xpointer<void> Field18;
            int Field1C;
            char Field20;
        };

        SWA_INSERT_PADDING(0x94);
        xpointer<SSaveData> m_pSaveData;
        xpointer<SStageParameter> m_pStageParameter;
    };
    SWA_ASSERT_OFFSETOF(CGameParameter::SStageParameter, Field00, 0x00);
    SWA_ASSERT_OFFSETOF(CGameParameter::SStageParameter, FieldC, 0x0C);
    SWA_ASSERT_OFFSETOF(CGameParameter::SStageParameter, Field10, 0x10);
    SWA_ASSERT_OFFSETOF(CGameParameter::SStageParameter, Field14, 0x14);
    SWA_ASSERT_OFFSETOF(CGameParameter::SStageParameter, Field18, 0x18);
    SWA_ASSERT_OFFSETOF(CGameParameter::SStageParameter, Field1C, 0x1C);
    SWA_ASSERT_OFFSETOF(CGameParameter::SStageParameter, Field20, 0x20);    
    SWA_ASSERT_OFFSETOF(CGameParameter, m_pSaveData, 0x94);
    SWA_ASSERT_OFFSETOF(CGameParameter, m_pStageParameter, 0x98);
}
