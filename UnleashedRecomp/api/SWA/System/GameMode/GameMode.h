#pragma once

#include <SWA.inl>

namespace SWA
{
    class CGameMode // : Hedgehog::Universe::TStateMachine<SWA::CGame>::TState
    {
    public:
        SWA_INSERT_PADDING(0x60); // base
        SWA_INSERT_PADDING(0x08);
    };
}
