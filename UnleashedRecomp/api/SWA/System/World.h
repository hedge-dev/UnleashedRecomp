#pragma once

#include "SWA.inl"
#include "Hedgehog/Base/Thread/hhSynchronizedObject.h"
#include "Hedgehog/Universe/Engine/hhMessageActor.h"

namespace SWA
{
    class CWorld : public Hedgehog::Base::CSynchronizedObject, public Hedgehog::Universe::CMessageActor
    {
    public:
        class CMember
        {
        public:
            SWA_INSERT_PADDING(0x80);
        };

        xpointer<CMember> m_pMember;
    };
}
