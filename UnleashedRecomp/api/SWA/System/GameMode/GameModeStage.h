#pragma once

#include <SWA.inl>

namespace SWA
{
    class CGameModeStage : public CGameMode
    {
    public:
        xpointer<void> m_pVftable;
        SWA_INSERT_PADDING(0x1B4);
    };
}
