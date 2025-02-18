#pragma once

#include <SWA.inl>

namespace SWA::Message
{
    class MsgSetPosition : public Hedgehog::Universe::MessageTypeSet
    {
    public:
        SWA_INSERT_PADDING(0x10);
        Hedgehog::Math::CVector m_Position;

        MsgSetPosition(const Hedgehog::Math::CVector& in_rPosition) : m_Position(in_rPosition) {}
    };

    SWA_ASSERT_OFFSETOF(MsgSetPosition, m_Position, 0x20);
    SWA_ASSERT_SIZEOF(MsgSetPosition, 0x30);
}
