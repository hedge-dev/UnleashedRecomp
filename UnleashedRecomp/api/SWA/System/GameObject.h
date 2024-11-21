#pragma once

#include "SWA.inl"
#include "Hedgehog/Base/Thread/hhSynchronizedPtr.h"
#include "Hedgehog/Universe/Engine/hhMessageActor.h"
#include "Hedgehog/Universe/Engine/hhUpdateUnit.h"

namespace SWA
{
    class CGameDocument;
    class CWorld;

    class CGameObject : public Hedgehog::Universe::CUpdateUnit, public Hedgehog::Universe::CMessageActor
    {
    public:
        class CMember
        {
        public:
            Hedgehog::Base::TSynchronizedPtr<CGameDocument> m_pGameDocument;
            Hedgehog::Base::TSynchronizedPtr<CWorld> m_pWorld;
            SWA_INSERT_PADDING(0x70);
        };

        xpointer<void> m_pVftable;
        xpointer<CMember> m_pMember;
    };
}
