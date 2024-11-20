#pragma once

#include <SWA.inl>
#include <SWA/Sound/SoundBGMBase.h>

namespace SWA
{
    class CSoundBGMActSonic : public CSoundBGMBase
    {
    public:
        class CMember
        {
        public:
            SWA_INSERT_PADDING(0x58);
            be<float> m_Volume1;
            be<float> m_Volume2;
        };

        xpointer<CMember> m_pMember;
    };
}
