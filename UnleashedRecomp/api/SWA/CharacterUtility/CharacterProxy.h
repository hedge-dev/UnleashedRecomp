#pragma once

#include <Hedgehog/Math/Vector.h>

namespace SWA
{
    class CCharacterProxy
    {
    public:
        SWA_INSERT_PADDING(0x120);
        Hedgehog::Math::CVector m_Position;
        Hedgehog::Math::CVector m_Velocity;
    };
}
