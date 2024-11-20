#pragma once

#include <SWA.inl>

namespace SWA
{
    class CSoundBGMBase : Hedgehog::Universe::CMessageActor
    {
    public:
        SWA_INSERT_PADDING(0x04); // vftable ptr
        SWA_INSERT_PADDING(0x04);
        xpointer<CGameDocument> m_pGameDocument;
        bool m_Unk;
    };
}
