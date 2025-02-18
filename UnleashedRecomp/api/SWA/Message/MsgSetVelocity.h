#pragma once

#include <SWA.inl>

namespace SWA::Message
{
    class MsgSetVelocity : public Hedgehog::Universe::MessageTypeSet
    {
    public:
        SWA_INSERT_PADDING(0x10);
        Hedgehog::Math::CVector m_Velocity;

        MsgSetVelocity(const Hedgehog::Math::CVector& in_rVelocity) : m_Velocity(in_rVelocity) {}
    };

    SWA_ASSERT_OFFSETOF(MsgSetVelocity, m_Velocity, 0x20);
    SWA_ASSERT_SIZEOF(MsgSetVelocity, 0x30);
}
