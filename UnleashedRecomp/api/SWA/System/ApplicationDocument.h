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
    class CLoadPoolMemory;
}

namespace Hedgehog::Mirage
{
    class CMatrixNode;
    class CRenderScene;
    class CRenderingDevice;
}

namespace Hedgehog::Universe
{
    class CParallelJobManagerD3D9;
    class CMessageManager;
    class CUpdateManager;
}

namespace SWA
{
    class CApplication;
    class CDatabaseTree;
    class CReplayManager;
    class CItemParamManager;
    class CKyojuParamManager;
    class CHotdogSaveManager;
    class CStageListManager;
    class CStageListManager;
    class CSkillParamManager;
    class CDLCManager;

    namespace Player
    {
        class CEvilAttackAction;
    };

    namespace Sequence::Core
    {
        class CManager;
    };  

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
            xpointer<Hedgehog::Universe::CMessageManager> m_pMessageManager;
            SWA_INSERT_PADDING(0x8);
            boost::shared_ptr<Hedgehog::Universe::CUpdateManager> m_spUpdateManager;
            xpointer<CGame> m_pGame;
            SWA_INSERT_PADDING(0x4);
            boost::shared_ptr<Hedgehog::Mirage::CRenderingDevice> m_spRenderingDevice;
            SWA_INSERT_PADDING(0xC);
            boost::shared_ptr<Hedgehog::Database::CDatabase> m_spInspireDatabase;
            boost::anonymous_shared_ptr m_spField44;
            SWA_INSERT_PADDING(0x28);
            Hedgehog::Base::CSharedString m_Field74;
            SWA_INSERT_PADDING(0xC);
            boost::shared_ptr<Hedgehog::Mirage::CMatrixNode> m_spMatrixNodeRoot;
            xpointer<void> m_pFlagManager;
            xpointer<void> m_pField8C;
            boost::shared_ptr<CReplayManager> m_spReplayManager;
            SWA_INSERT_PADDING(0x4);
            CGammaController m_GammaController;
            boost::shared_ptr<CLoading> m_spLoading;
            boost::shared_ptr<CSaveIcon> m_spSaveIcon;
            SWA_INSERT_PADDING(0x4);
            boost::shared_ptr<Sequence::Core::CManager> m_spSequenceCore;
            xpointer<Achievement::CManager> m_pAchievementManager;
            SWA_INSERT_PADDING(0x4);
            boost::shared_ptr<CDatabaseTree> m_spDatabaseTree;
            Hedgehog::Base::CSharedString m_Field10C;
            SWA_INSERT_PADDING(0x20);
            boost::shared_ptr<Hedgehog::Mirage::CRenderScene> m_spRenderScene;
            boost::shared_ptr<CGameParameter> m_spGameParameter;
            SWA_INSERT_PADDING(0xC);
            boost::shared_ptr<CItemParamManager> m_spItemParameter;
            boost::shared_ptr<CKyojuParamManager> m_spKyojuParameter;
            boost::shared_ptr<CHotdogSaveManager> m_spHotdogSaveManager;
            boost::shared_ptr<CStageListManager> m_spStageListManager;
            boost::shared_ptr<Player::CEvilAttackAction> m_spEvilAttackActionDB;
            boost::shared_ptr<CSkillParamManager> m_spSkillParamManager;
            SWA_INSERT_PADDING(0x2C);
            boost::shared_ptr<Hedgehog::Database::CLoadPoolMemory> m_spDatabaseLoadPoolMemory;
            SWA_INSERT_PADDING(0x8);
            boost::shared_ptr<Hedgehog::Base::CCriticalSection> m_spCriticalSection;
            SWA_INSERT_PADDING(0xC);
            boost::shared_ptr<CDLCManager> m_spDLCManager;
            SWA_INSERT_PADDING(0xC);
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
    SWA_ASSERT_OFFSETOF(CApplicationDocument::CMember, m_pMessageManager, 0xC);
    SWA_ASSERT_OFFSETOF(CApplicationDocument::CMember, m_spUpdateManager, 0x18);
    SWA_ASSERT_OFFSETOF(CApplicationDocument::CMember, m_pGame, 0x20);
    SWA_ASSERT_OFFSETOF(CApplicationDocument::CMember, m_spRenderingDevice, 0x28);
    SWA_ASSERT_OFFSETOF(CApplicationDocument::CMember, m_spInspireDatabase, 0x3C);
    SWA_ASSERT_OFFSETOF(CApplicationDocument::CMember, m_spField44, 0x44);
    SWA_ASSERT_OFFSETOF(CApplicationDocument::CMember, m_Field74, 0x74);
    SWA_ASSERT_OFFSETOF(CApplicationDocument::CMember, m_spMatrixNodeRoot, 0x84);
    SWA_ASSERT_OFFSETOF(CApplicationDocument::CMember, m_spReplayManager, 0x94);
    SWA_ASSERT_OFFSETOF(CApplicationDocument::CMember, m_GammaController, 0xA0);
    SWA_ASSERT_OFFSETOF(CApplicationDocument::CMember, m_spLoading, 0xE0);
    SWA_ASSERT_OFFSETOF(CApplicationDocument::CMember, m_spSaveIcon, 0xE8);
    SWA_ASSERT_OFFSETOF(CApplicationDocument::CMember, m_spSequenceCore, 0xF4);
    SWA_ASSERT_OFFSETOF(CApplicationDocument::CMember, m_pAchievementManager, 0xFC);
    SWA_ASSERT_OFFSETOF(CApplicationDocument::CMember, m_spDatabaseTree, 0x104);
    SWA_ASSERT_OFFSETOF(CApplicationDocument::CMember, m_Field10C, 0x10C);
    SWA_ASSERT_OFFSETOF(CApplicationDocument::CMember, m_spRenderScene, 0x130);
    SWA_ASSERT_OFFSETOF(CApplicationDocument::CMember, m_spGameParameter, 0x138);
    SWA_ASSERT_OFFSETOF(CApplicationDocument::CMember, m_spItemParameter, 0x14C);
    SWA_ASSERT_OFFSETOF(CApplicationDocument::CMember, m_spKyojuParameter, 0x154);
    SWA_ASSERT_OFFSETOF(CApplicationDocument::CMember, m_spHotdogSaveManager, 0x15C);
    SWA_ASSERT_OFFSETOF(CApplicationDocument::CMember, m_spStageListManager, 0x164);
    SWA_ASSERT_OFFSETOF(CApplicationDocument::CMember, m_spEvilAttackActionDB, 0x16C);
    SWA_ASSERT_OFFSETOF(CApplicationDocument::CMember, m_spSkillParamManager, 0x174);
    SWA_ASSERT_OFFSETOF(CApplicationDocument::CMember, m_spDatabaseLoadPoolMemory, 0x1A8);
    SWA_ASSERT_OFFSETOF(CApplicationDocument::CMember, m_spCriticalSection, 0x1B8);
    SWA_ASSERT_OFFSETOF(CApplicationDocument::CMember, m_spDLCManager, 0x1CC);
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
