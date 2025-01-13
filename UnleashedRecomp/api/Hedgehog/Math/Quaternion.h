#pragma once

#include <SWA.inl>

namespace Hedgehog::Math
{
    class CQuaternion //Eigen::Quaternionf;
    {
    public:
        be<float> X;
        be<float> Y;
        be<float> Z;
        be<float> W;
    };

    SWA_ASSERT_SIZEOF(CQuaternion, 0x10);
}
