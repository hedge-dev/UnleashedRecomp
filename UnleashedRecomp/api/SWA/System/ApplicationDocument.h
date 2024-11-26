#pragma once

#include <SWA.inl>
#include <SWA/System/Game.h>

namespace SWA
{
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
            SWA_INSERT_PADDING(0x20);
            boost::shared_ptr<CGame> m_pGame;
            SWA_INSERT_PADDING(0xD4);
            xpointer<Achievement::CManager> m_pAchievementManager;
            SWA_INSERT_PADDING(0x3C);
            xpointer<void> m_spGameParameter;
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
    };
}

#include "ApplicationDocument.inl"
