#pragma once

#include <SWA.inl>

namespace SWA
{
    class CExPlayerTails
    {
    public:
        class CExPlayerTailsStateContext
        {
        public:
            SWA_INSERT_PADDING(0x1F8);
            be<float> m_SplineBossStart;
            be<float> m_SplineEnd;
            SWA_INSERT_PADDING(0x30);
            be<float> m_SplineProgress;
            SWA_INSERT_PADDING(0x10);
            xpointer<void> m_Field244;
            SWA_INSERT_PADDING(0x18);
            be<float> m_SplineSpeed;
            SWA_INSERT_PADDING(0x0C);
            be<uint32_t> m_State; // 0 - Intro; 1 - Boss Intro; 3 - Boss
        };

        class CStateBase : public Hedgehog::Universe::CStateMachineBase::CStateBase {};
    };

    SWA_ASSERT_OFFSETOF(CExPlayerTails::CExPlayerTailsStateContext, m_SplineBossStart, 0x1F8);
    SWA_ASSERT_OFFSETOF(CExPlayerTails::CExPlayerTailsStateContext, m_SplineEnd, 0x1FC);
    SWA_ASSERT_OFFSETOF(CExPlayerTails::CExPlayerTailsStateContext, m_SplineProgress, 0x230);
    SWA_ASSERT_OFFSETOF(CExPlayerTails::CExPlayerTailsStateContext, m_Field244, 0x244);
    SWA_ASSERT_OFFSETOF(CExPlayerTails::CExPlayerTailsStateContext, m_SplineSpeed, 0x260);
    SWA_ASSERT_OFFSETOF(CExPlayerTails::CExPlayerTailsStateContext, m_State, 0x270);
}
