#pragma once

#include <SWA.inl>

namespace SWA
{
    class CGameMode : public Hedgehog::Universe::CStateMachineBase::CStateBase
    {
    public:
        SWA_INSERT_PADDING(0x08);
    };
}
