#pragma once

#include <SWA.inl>

namespace Hedgehog::Math
{
    class CMatrix
    {
    public:
        be<float> data[16]; // Eigen::Affine3f
    };

    class CMatrix44
    {
    public:
        be<float> data[16]; // Eigen::Matrix4f
    };

    SWA_ASSERT_SIZEOF(CMatrix, 0x40);
    SWA_ASSERT_SIZEOF(CMatrix44, 0x40);
}
