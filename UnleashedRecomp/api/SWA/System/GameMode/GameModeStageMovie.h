#pragma once

#include <SWA.inl>

namespace SWA
{
    class CGameModeStageMovie : public CGameModeStage
    {
    public:
        class CRender : public Hedgehog::Mirage::CRenderable
        {
        public:
            xpointer<CGameModeStageMovie> m_pThis;
        };

        xpointer<void> m_pVftable;
        SWA_INSERT_PADDING(0x224);
        xpointer<CRender> m_pRender;
    };
}
