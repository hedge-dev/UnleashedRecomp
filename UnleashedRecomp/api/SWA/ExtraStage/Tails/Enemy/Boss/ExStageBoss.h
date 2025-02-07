#pragma once

#include <SWA.inl>

namespace SWA
{
    class CExStageBoss
    {
    public:
        class CStateBase;
        class CStateBattle;

        class CExStageBossStateContext
        {
        public:
            SWA_INSERT_PADDING(0x14C);
            be<float> m_SplineProgress;
            SWA_INSERT_PADDING(0x0C);
            be<float> m_SplineSpeed;
            SWA_INSERT_PADDING(0x28);
            be<float> m_Field188;
            be<float> m_Field18C;
            SWA_INSERT_PADDING(0x21);
            bool m_IsBattleStart;
            SWA_INSERT_PADDING(0x36E);
            be<float> m_Field520;
        };
    };

    SWA_ASSERT_OFFSETOF(CExStageBoss::CExStageBossStateContext, m_SplineProgress, 0x14C);
    SWA_ASSERT_OFFSETOF(CExStageBoss::CExStageBossStateContext, m_SplineSpeed, 0x15C);
    SWA_ASSERT_OFFSETOF(CExStageBoss::CExStageBossStateContext, m_Field188, 0x188);
    SWA_ASSERT_OFFSETOF(CExStageBoss::CExStageBossStateContext, m_Field18C, 0x18C);
    SWA_ASSERT_OFFSETOF(CExStageBoss::CExStageBossStateContext, m_IsBattleStart, 0x1B1);
    SWA_ASSERT_OFFSETOF(CExStageBoss::CExStageBossStateContext, m_Field520, 0x520);
}
