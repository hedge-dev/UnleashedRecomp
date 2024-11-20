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
            SWA_INSERT_PADDING(0x15C);
            be<uint32_t> m_Score;
        };

        // TODO: Hedgehog::Base::TSynchronizedPtr<CGameDocument>
        static CGameDocument* GetInstance();

        SWA_INSERT_PADDING(0x04); // vftable ptr
        xpointer<CMember> m_pMember;
    };
}

#include "GameDocument.inl"
