#pragma once

#include <SWA.inl>

namespace Hedgehog::Mirage
{
    class CCamera : Base::CObject
    {
    public:
        Math::CMatrix m_View;
        Math::CMatrix44 m_Projection;
        Math::CVector m_Position;
        Math::CVector m_Direction;
        be<float> m_AspectRatio;
        be<float> m_Near;
        be<float> m_Far;
    };
}
