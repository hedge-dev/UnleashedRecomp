#pragma once

#include <SWA.inl>
#include <SWA/System/GameObject.h>

namespace SWA
{
    class CCamera : public CGameObject // , public Hedgehog::Universe::TStateMachine<CCamera>
    {
    public:
        xpointer<void> m_pVftable;
        SWA_INSERT_PADDING(0xC4);
        be<float> m_VertAspectRatio;
        SWA_INSERT_PADDING(0x48);
        be<float> m_HorzAspectRatio;
        SWA_INSERT_PADDING(0x178);
        be<float> m_FieldOfView;
        be<float> m_VertFieldOfView;
        be<float> m_HorzFieldOfView;
        SWA_INSERT_PADDING(0x18);
        bool m_InvertY;
        bool m_InvertX;
    };
}
