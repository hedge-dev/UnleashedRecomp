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

        CVector operator*(const float& scalar) const
        {
            return { X * scalar, Y * scalar, Z * scalar };
        }

        CVector operator+(const CVector& v) const
        {
            return { X + v.X, Y + v.Y, Z + v.Z };
        }
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
