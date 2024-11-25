#pragma once

#include <SWA.inl>

namespace SWA
{
    class CHudSonicStage
    {
    public:
        SWA_INSERT_PADDING(0xE0);
        Chao::CSD::RCPtr<Chao::CSD::CProject> m_rcPlayScreen;
        Chao::CSD::RCPtr<Chao::CSD::CScene> m_rcSpeedGauge;
        Chao::CSD::RCPtr<Chao::CSD::CScene> m_rcRingEnergyGauge;
        Chao::CSD::RCPtr<Chao::CSD::CScene> m_rcGaugeFrame;
        SWA_INSERT_PADDING(0x28);
        Chao::CSD::RCPtr<Chao::CSD::CNode> m_rcScoreCount;
        Chao::CSD::RCPtr<Chao::CSD::CNode> m_rcTimeCount;
        Chao::CSD::RCPtr<Chao::CSD::CNode> m_rcTimeCount2;
        Chao::CSD::RCPtr<Chao::CSD::CNode> m_rcTimeCount3;
        Chao::CSD::RCPtr<Chao::CSD::CNode> m_rcPlayerCount;
    };
}
