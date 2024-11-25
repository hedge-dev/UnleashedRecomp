#pragma once

#include <SWA.inl>
#include <SWA/Sound/SoundBGMBase.h>

namespace SWA
{
    class CSoundAdministrator : public CGameObject
    {
    public:
        class CSimplePlayer {};

        class CBgm
        {
        public:
            xpointer<CSoundAdministrator> m_pThis;
            boost::shared_ptr<CSoundBGMBase> m_spSoundBGM;
            be<float> m_Volume1;
            be<float> m_Volume2;
            be<float> m_Volume3;
            be<float> m_Volume4;
            SWA_INSERT_PADDING(0x10);
        };

        class CCommunicator
        {
        public:
            class CCommunicatorDevice : Hedgehog::Base::CSynchronizedObject
            {
            public:
                SWA_INSERT_PADDING(0x08);
                Hedgehog::Base::CSharedString m_HostName;
                SWA_INSERT_PADDING(0x04);
                Hedgehog::Base::CSharedString m_Category;
                Hedgehog::Base::CSharedString m_Command;
                SWA_INSERT_PADDING(0x04);
            };

            xpointer<CSoundAdministrator> m_pThis;
            boost::shared_ptr<CCommunicatorDevice> m_spCommunicatorDevice;
        };

        class CMember
        {
        public:
            boost::shared_ptr<CSimplePlayer> m_spSimplePlayer;
            boost::shared_ptr<CBgm> m_spBgm;
            boost::shared_ptr<CCommunicator> m_spCommunicator;
            SWA_INSERT_PADDING(0x58);
        };

        xpointer<void> m_pVftable;
        SWA_INSERT_PADDING(0x04);
        xpointer<CMember> m_pMember;
    };
}
