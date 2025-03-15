#pragma once

#include <SWA.inl>
#include <SWA/System/Game.h>
#include <SWA/System/GameParameter.h>
#include <SWA/System/GammaController.h>

namespace Hedgehog::Base
{
    class CCriticalSection;
}

namespace Hedgehog::Database
{
    class CDatabase;
}

namespace Hedgehog::Mirage
{
    class CMatrixNode;
    class CRenderScene;
}

namespace Hedgehog::Universe
{
    class CParallelJobManagerD3D9;
}

namespace SWA
{
    class CApplication;
    class CDatabaseTree;

    enum ELanguage : uint32_t
    {
        eLanguage_English,
        eLanguage_Japanese,
        eLanguage_German,
        eLanguage_French,
        eLanguage_Italian,
        eLanguage_Spanish
    };

    enum EVoiceLanguage : uint32_t
    {
        eVoiceLanguage_English,
        eVoiceLanguage_Japanese
    };

    enum ERegion : uint32_t
    {
        eRegion_Japan,
        eRegion_RestOfWorld
    };

    class CApplicationDocument : public Hedgehog::Base::CSynchronizedObject
    {
    public:
        class CMember
        {
        public:
            xpointer<CApplication> m_pApplication;
            boost::shared_ptr<Hedgehog::Universe::CParallelJobManagerD3D9> m_spParallelJobManagerD3D9;
            SWA_INSERT_PADDING(0x14);
            boost::shared_ptr<CGame> m_spGame;
            SWA_INSERT_PADDING(0x14);
            boost::shared_ptr<Hedgehog::Database::CDatabase> m_spInspireDatabase;
            SWA_INSERT_PADDING(0x30);
            Hedgehog::Base::CSharedString m_Field74;
            SWA_INSERT_PADDING(0x0C);
            boost::shared_ptr<Hedgehog::Mirage::CMatrixNode> m_spMatrixNodeRoot;
            SWA_INSERT_PADDING(0x14);
            CGammaController m_GammaController;
            boost::shared_ptr<CLoading> m_spLoading;
            SWA_INSERT_PADDING(0x14);
            boost::shared_ptr<Achievement::CManager> m_spAchievementManager;
            boost::shared_ptr<CDatabaseTree> m_spDatabaseTree;
            Hedgehog::Base::CSharedString m_Field10C;
            SWA_INSERT_PADDING(0x1C);
            boost::shared_ptr<Hedgehog::Mirage::CRenderScene> m_spRenderScene;
            SWA_INSERT_PADDING(0x04);
            boost::shared_ptr<CGameParameter> m_spGameParameter;
            SWA_INSERT_PADDING(0x0C);
            boost::anonymous_shared_ptr m_spItemParamManager;
            SWA_INSERT_PADDING(0x64);
            boost::shared_ptr<Hedgehog::Base::CCriticalSection> m_spCriticalSection;
            SWA_INSERT_PADDING(0x14);
            bool m_ShowDLCInfo;
            SWA_INSERT_PADDING(0x08);
        };

        // TODO: Hedgehog::Base::TSynchronizedPtr<CApplicationDocument>
        static CApplicationDocument* GetInstance();

        xpointer<CMember> m_pMember;
        be<ELanguage> m_Language;
        be<EVoiceLanguage> m_VoiceLanguage;
        SWA_INSERT_PADDING(0x08);
        be<ERegion> m_Region;
        bool m_InspireVoices;
        bool m_InspireSubtitles;
        SWA_INSERT_PADDING(0x28);
    };

    SWA_ASSERT_OFFSETOF(CApplicationDocument::CMember, m_pApplication, 0x00);
    SWA_ASSERT_OFFSETOF(CApplicationDocument::CMember, m_spParallelJobManagerD3D9, 0x04);
    SWA_ASSERT_OFFSETOF(CApplicationDocument::CMember, m_spGame, 0x20);
    SWA_ASSERT_OFFSETOF(CApplicationDocument::CMember, m_spInspireDatabase, 0x3C);
    SWA_ASSERT_OFFSETOF(CApplicationDocument::CMember, m_Field74, 0x74);
    SWA_ASSERT_OFFSETOF(CApplicationDocument::CMember, m_spMatrixNodeRoot, 0x84);
    SWA_ASSERT_OFFSETOF(CApplicationDocument::CMember, m_GammaController, 0xA0);
    SWA_ASSERT_OFFSETOF(CApplicationDocument::CMember, m_spLoading, 0xE0);
    SWA_ASSERT_OFFSETOF(CApplicationDocument::CMember, m_spAchievementManager, 0xFC);
    SWA_ASSERT_OFFSETOF(CApplicationDocument::CMember, m_spDatabaseTree, 0x104);
    SWA_ASSERT_OFFSETOF(CApplicationDocument::CMember, m_Field10C, 0x10C);
    SWA_ASSERT_OFFSETOF(CApplicationDocument::CMember, m_spRenderScene, 0x12C);
    SWA_ASSERT_OFFSETOF(CApplicationDocument::CMember, m_spGameParameter, 0x138);
    SWA_ASSERT_OFFSETOF(CApplicationDocument::CMember, m_spItemParamManager, 0x14C);
    SWA_ASSERT_OFFSETOF(CApplicationDocument::CMember, m_spCriticalSection, 0x1B8);
    SWA_ASSERT_OFFSETOF(CApplicationDocument::CMember, m_ShowDLCInfo, 0x1D4);
    SWA_ASSERT_SIZEOF(CApplicationDocument::CMember, 0x1E0);

    SWA_ASSERT_OFFSETOF(CApplicationDocument, m_pMember, 0x04);
    SWA_ASSERT_OFFSETOF(CApplicationDocument, m_Language, 0x08);
    SWA_ASSERT_OFFSETOF(CApplicationDocument, m_VoiceLanguage, 0x0C);
    SWA_ASSERT_OFFSETOF(CApplicationDocument, m_Region, 0x18);
    SWA_ASSERT_OFFSETOF(CApplicationDocument, m_InspireVoices, 0x1C);
    SWA_ASSERT_OFFSETOF(CApplicationDocument, m_InspireSubtitles, 0x1D);
    SWA_ASSERT_SIZEOF(CApplicationDocument, 0x48);
}

#include "ApplicationDocument.inl"
