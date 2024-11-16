#pragma once

#include "SWA/Sound/Sound.h"

namespace Hedgehog::Database
{
    class CDatabase;
}

namespace SWA
{
    class CGameDocument : public Hedgehog::Base::CSynchronizedObject
    {
    public:
        class CMember
        {
        public:
            SWA_INSERT_PADDING(0x1C);
            boost::shared_ptr<Hedgehog::Database::CDatabase> m_spDatabase;
            SWA_INSERT_PADDING(0x90);
            xpointer<CSound> m_pSound;
            SWA_INSERT_PADDING(0x158);
            be<uint32_t> m_Score;
        };

        // TODO: Hedgehog::Base::TSynchronizedPtr<CGameDocument>
        static CGameDocument* GetInstance();

        SWA_INSERT_PADDING(0x04);
        xpointer<CMember> m_pMember;
    };
}

#include "GameDocument.inl"
