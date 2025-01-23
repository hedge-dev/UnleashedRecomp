#pragma once

#include "SWA.inl"

namespace Hedgehog::Math
{
    class CVector2
    {
    public:
        be<float> X;
        be<float> Y;
    };

    class CVector
    {
    public:
        be<float> X;
        be<float> Y;
        be<float> Z;
        be<float> W;
    };

    class CVector4
    {
    public:
        be<float> X;
        be<float> Y;
        be<float> Z;
        be<float> W;
    };
}
