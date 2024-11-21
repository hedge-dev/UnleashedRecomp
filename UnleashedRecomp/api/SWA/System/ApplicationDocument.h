#pragma once

#include <SWA.inl>
#include <SWA/System/Game.h>

namespace SWA
{
    enum EVoiceLanguage : uint32_t
    {
        eVoiceLanguage_English,
        eVoiceLanguage_Japanese
    };

    class CApplicationDocument // : public Hedgehog::Base::CSynchronizedObject
    {
    public:
        class CMember
        {
        public:
            SWA_INSERT_PADDING(0x20);
            boost::shared_ptr<CGame> m_pGame;
            SWA_INSERT_PADDING(0x114);
            xpointer<void> m_spGameParameter;
        };

        // TODO: Hedgehog::Base::TSynchronizedPtr<CApplicationDocument>
        static CApplicationDocument* GetInstance();

        xpointer<void> m_pVftable;
        xpointer<CMember> m_pMember;
        SWA_INSERT_PADDING(0x04);
        be<EVoiceLanguage> m_VoiceLanguage;
        SWA_INSERT_PADDING(0x0C);
        be<uint32_t> m_Region;
    };
}

#include "ApplicationDocument.inl"
