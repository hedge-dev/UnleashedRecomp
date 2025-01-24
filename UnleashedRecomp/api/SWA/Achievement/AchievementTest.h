#pragma once

#include <SWA.inl>
#include <SWA/Achievement/AchievementID.h>

namespace SWA
{
    class CAchievementTest
    {
    public:
        SWA_INSERT_PADDING(0x38);
        be<uint32_t> m_Unk1;
        be<EAchievementID> m_AchievementID;
        uint8_t m_Unk2;
    };
}
