#pragma once

#include <SWA.inl>

namespace SWA
{
    class CTitleStateBase : public Hedgehog::Universe::CStateMachineBase::CStateBase
    {
    public:
        class CTitleStateContext
        {
        public:
            SWA_INSERT_PADDING(0x1E8);
            xpointer<CTitleMenu> m_pTitleMenu;
        };
    };
}
