#pragma once

#include "SWA.inl"
#include "SWA/Player/Character/EvilSonic/EvilSonicContext.h"

namespace SWA::Player
{
    class CEvilSonic // : public CPlayer
    {
    public:
        SWA_INSERT_PADDING(0xC4);
        xpointer<CEvilSonicContext> m_spContext;
    };
}
