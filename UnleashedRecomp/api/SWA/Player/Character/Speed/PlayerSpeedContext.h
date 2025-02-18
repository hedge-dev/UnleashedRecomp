#pragma once

#include <SWA.inl>

namespace SWA::Player
{
    class CPlayerSpeedContext : public CPlayerContext
    {
    public:
        // TODO: Hedgehog::Base::TSynchronizedPtr<CGameDocument>
        static CPlayerSpeedContext* GetInstance();
    };
}

#include "PlayerSpeedContext.inl"
