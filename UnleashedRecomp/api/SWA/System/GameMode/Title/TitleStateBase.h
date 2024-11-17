#pragma once

#include <SWA.inl>

namespace SWA
{
    class CTitleStateBase // : Hedgehog::Universe::TStateMachine<SWA::CTitleManager>::TState
    {
    public:
        class CMember
        {
        public:
            SWA_INSERT_PADDING(0x1E8);
            xpointer<CTitleMenu> m_pTitleMenu;
        };

        SWA_INSERT_PADDING(0x08);
        xpointer<CMember> m_pMember;
        SWA_INSERT_PADDING(0x5C);
        be<uint32_t> m_State;
    };
}
