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
            struct SScoreInfo
            {
                be<uint32_t> SRank;
                be<uint32_t> ARank;
                be<uint32_t> BRank;
                be<uint32_t> CRank;
                be<uint32_t> DRank;
                SWA_INSERT_PADDING(0x0C);
                be<float> PointMarkerRecordSpeed;
                SWA_INSERT_PADDING(0x0C);
                be<uint32_t> PointMarkerCount;
                be<uint32_t> EnemyScore;
                be<uint32_t> TrickScore;
                SWA_INSERT_PADDING(0x10);
            };

            SWA_INSERT_PADDING(0x1C);
            boost::shared_ptr<Hedgehog::Database::CDatabase> m_spDatabase;
            SWA_INSERT_PADDING(0x88);
            Hedgehog::Base::CSharedString m_StageName;
            xpointer<CSoundAdministrator> m_pSoundAdministrator;
            SWA_INSERT_PADDING(0x48);
            xpointer<CGeneralWindow> m_pGeneralWindow;
            SWA_INSERT_PADDING(0xD8);
            SScoreInfo m_ScoreInfo;
            SWA_INSERT_PADDING(0x0C);
        };

        // TODO: Hedgehog::Base::TSynchronizedPtr<CGameDocument>
        static CGameDocument* GetInstance();

        xpointer<void> m_pVftable;
        xpointer<CMember> m_pMember;
    };

    SWA_ASSERT_OFFSETOF(CGameDocument::CMember::SScoreInfo, SRank, 0x00);
    SWA_ASSERT_OFFSETOF(CGameDocument::CMember::SScoreInfo, ARank, 0x04);
    SWA_ASSERT_OFFSETOF(CGameDocument::CMember::SScoreInfo, BRank, 0x08);
    SWA_ASSERT_OFFSETOF(CGameDocument::CMember::SScoreInfo, CRank, 0x0C);
    SWA_ASSERT_OFFSETOF(CGameDocument::CMember::SScoreInfo, DRank, 0x10);
    SWA_ASSERT_OFFSETOF(CGameDocument::CMember::SScoreInfo, PointMarkerRecordSpeed, 0x20);
    SWA_ASSERT_OFFSETOF(CGameDocument::CMember::SScoreInfo, PointMarkerCount, 0x30);
    SWA_ASSERT_OFFSETOF(CGameDocument::CMember::SScoreInfo, EnemyScore, 0x34);
    SWA_ASSERT_OFFSETOF(CGameDocument::CMember::SScoreInfo, TrickScore, 0x38);
    SWA_ASSERT_SIZEOF(CGameDocument::CMember::SScoreInfo, 0x4C);

    SWA_ASSERT_OFFSETOF(CGameDocument::CMember, m_spDatabase, 0x1C);
    SWA_ASSERT_OFFSETOF(CGameDocument::CMember, m_StageName, 0xAC);
    SWA_ASSERT_OFFSETOF(CGameDocument::CMember, m_pSoundAdministrator, 0xB0);
    SWA_ASSERT_OFFSETOF(CGameDocument::CMember, m_pGeneralWindow, 0xFC);
    SWA_ASSERT_OFFSETOF(CGameDocument::CMember, m_ScoreInfo, 0x1D8);
    SWA_ASSERT_SIZEOF(CGameDocument::CMember, 0x230);

    SWA_ASSERT_OFFSETOF(CGameDocument, m_pMember, 0x08);
    SWA_ASSERT_SIZEOF(CGameDocument, 0x0C);
}

#include "GameDocument.inl"
