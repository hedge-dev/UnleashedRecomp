#pragma once

#include "SWA.inl"
#include "SWA/Player/Character/EvilSonic/Hud/EvilHudGuide.h"

namespace SWA::Player
{
    class CEvilSonicContext : public CPlayerContext
    {
    public:
        SWA_INSERT_PADDING(0x478);
        be<float> m_DarkGaiaEnergy;
        SWA_INSERT_PADDING(0x138);
        be<uint32_t> m_AnimationID;
        SWA_INSERT_PADDING(0x38);
        be<float> m_Field800;    // Related to EvilHudGuide
        be<uint32_t> m_Field804; // Related to EvilHudGuide
        SWA_INSERT_PADDING(0x18);
        be<EGuideType> m_GuideType;
        SWA_INSERT_PADDING(0xA8);
        be<uint32_t> m_OutOfControlCount;
    };

    SWA_ASSERT_OFFSETOF(CEvilSonicContext, m_DarkGaiaEnergy, 0x688);
    SWA_ASSERT_OFFSETOF(CEvilSonicContext, m_AnimationID, 0x7C4);
    SWA_ASSERT_OFFSETOF(CEvilSonicContext, m_Field800, 0x800);
    SWA_ASSERT_OFFSETOF(CEvilSonicContext, m_Field804, 0x804);
    SWA_ASSERT_OFFSETOF(CEvilSonicContext, m_GuideType, 0x820);
    SWA_ASSERT_OFFSETOF(CEvilSonicContext, m_OutOfControlCount, 0x8CC);
}
