#pragma once

#include <SWA.inl>

namespace SWA::Message
{
    class MsgCameraPauseMove : public Hedgehog::Universe::MessageTypeSet
    {
    public:
        SWA_INSERT_PADDING(0x08);
        bool m_isPaused;

        MsgCameraPauseMove(bool in_isPaused) : m_isPaused(in_isPaused) {}
    };

    SWA_ASSERT_OFFSETOF(MsgCameraPauseMove, m_isPaused, 0x18);
}
