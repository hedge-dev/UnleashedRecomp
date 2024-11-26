#pragma once

#include <SWA.inl>

namespace SWA::Achievement
{
    class CManager : public Hedgehog::Universe::CUpdateUnit
    {
    public:
        class CMember
        {
        public:
            SWA_INSERT_PADDING(0x08);
            be<uint32_t> m_AchievementID;
        };

        SWA_INSERT_PADDING(0x98);
        xpointer<CMember> m_pMember;
        be<uint32_t> m_IsUnlocked;
        SWA_INSERT_PADDING(0x10);
    };
}
