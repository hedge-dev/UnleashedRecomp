#pragma once

#include <SWA.inl>
#include "boost/smart_ptr/shared_ptr.h"

namespace SWA::Player
{
    class CPlayer;

    class CPlayerContext
    {
    public:
        SWA_INSERT_PADDING(0x10);
        boost::shared_ptr<CMatrixNodeTransform> m_spMatrixNode;
        SWA_INSERT_PADDING(0x18);
        boost::anonymous_shared_ptr m_spRayCastCollision;
        SWA_INSERT_PADDING(0xC8);
        xpointer<CPlayer> m_pPlayer;
        SWA_INSERT_PADDING(0xF8);
        boost::shared_ptr<void> m_spParameter;
        SWA_INSERT_PADDING(0x0C);
    };

    SWA_ASSERT_OFFSETOF(CPlayerContext, m_spMatrixNode, 0x10);
    SWA_ASSERT_OFFSETOF(CPlayerContext, m_spRayCastCollision, 0x30);
    SWA_ASSERT_OFFSETOF(CPlayerContext, m_pPlayer, 0x100);
    SWA_ASSERT_OFFSETOF(CPlayerContext, m_spParameter, 0x1FC);
    SWA_ASSERT_SIZEOF(CPlayerContext, 0x210);
}
