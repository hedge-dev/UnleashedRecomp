#pragma once

#include "SWA.inl"

namespace SWA::Player
{
    enum EGuideAction : uint32_t
    {
        eGuideAction_Single,
        eGuideAction_Chain
    };

    enum EGuideType : uint32_t
    {
        eGuideType_A,
        eGuideType_B,
        eGuideType_X,
        eGuideType_Y
    };

    class CEvilHudGuide : public CGameObject
    {
    public:
        xpointer<void> m_pVftable;
        SWA_INSERT_PADDING(0x8D);
        bool m_IsShown;
        bool m_IsVisible;
        be<EGuideType> m_GuideType;
    };
}
