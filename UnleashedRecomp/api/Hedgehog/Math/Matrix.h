#pragma once

#include <SWA.inl>

namespace Hedgehog::Math
{
    class CMatrix
    {
    public:
        be<float> m_M00;
        be<float> m_M01;
        be<float> m_M02;
        be<float> m_M03;
        be<float> m_M10;
        be<float> m_M11;
        be<float> m_M12;
        be<float> m_M13;
        be<float> m_M20;
        be<float> m_M21;
        be<float> m_M22;
        be<float> m_M23;
        be<float> m_M30;
        be<float> m_M31;
        be<float> m_M32;
        be<float> m_M33;
    };
}
