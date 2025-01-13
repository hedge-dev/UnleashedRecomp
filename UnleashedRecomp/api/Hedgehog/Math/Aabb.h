#pragma once

#include <SWA.inl>

namespace Hedgehog::Math
{
    class CAabb // Eigen::AlignedBox3f
    {
    public:
        Hedgehog::Math::CVector min;
        Hedgehog::Math::CVector max;

        CVector Center() const;
    };

    SWA_ASSERT_SIZEOF(CAabb, 0x18);
}

#include "Aabb.inl"
