#pragma once

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
            SWA_INSERT_PADDING(0x8C);
            xpointer<CSoundAdministrator> m_pSoundAdministrator;
            SWA_INSERT_PADDING(0x158);
            be<uint32_t> m_Score;
        };

        // TODO: Hedgehog::Base::TSynchronizedPtr<CGameDocument>
        static CGameDocument* GetInstance();

        xpointer<void> m_pVftable;
        xpointer<CMember> m_pMember;
    };
}

#include "GameDocument.inl"
