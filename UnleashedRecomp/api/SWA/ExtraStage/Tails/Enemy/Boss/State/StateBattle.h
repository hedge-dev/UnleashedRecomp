#pragma once

#include <SWA.inl>
#include "SWA/ExtraStage/Tails/Enemy/Boss/ExStageBoss.h"

namespace SWA
{
    class CExStageBoss::CStateBattle : public CExStageBoss::CStateBase
    {
    public:
        SWA_INSERT_PADDING(0x08);
        be<float> m_Field68;
        be<float> m_FramesSinceLastMissile;
    };

    SWA_ASSERT_OFFSETOF(CExStageBoss::CStateBattle, m_Field68, 0x68);
    SWA_ASSERT_OFFSETOF(CExStageBoss::CStateBattle, m_FramesSinceLastMissile, 0x6C);
}
