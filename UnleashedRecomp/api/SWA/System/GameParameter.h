#pragma once

#include <SWA.inl>

namespace SWA
{
    class CGameParameter // : public Hedgehog::Universe::CMessageActor
    {
    public:
        struct SSaveData
        {
            SWA_INSERT_PADDING(0x8600);
            be<uint32_t> DLCFlags[8];
            SWA_INSERT_PADDING(0x15C);
        };

        struct SStageParameter;

        SWA_INSERT_PADDING(0x94);
        xpointer<SSaveData> m_pSaveData;
        xpointer<SStageParameter> m_pStageParameter;
    };

    SWA_ASSERT_OFFSETOF(CGameParameter::SSaveData, DLCFlags, 0x8600);
}
