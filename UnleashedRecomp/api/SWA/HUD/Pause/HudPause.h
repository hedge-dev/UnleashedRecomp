#pragma once

#include <SWA.inl>

using namespace Chao::CSD;

namespace SWA
{
    enum EActionType : uint32_t
    {
        eActionType_Undefined,
        eActionType_Status,
        eActionType_Return,
        eActionType_Inventory,
        eActionType_Skills,
        eActionType_Lab,
        eActionType_Wait,
        eActionType_Restart = 8,
        eActionType_Continue
    };

    enum EMenuType : uint32_t
    {
        eMenuType_WorldMap,
        eMenuType_Village,
        eMenuType_Stage,
        eMenuType_Hub,
        eMenuType_Misc
    };

    enum EStatusType : uint32_t
    {
        eStatusType_Idle,
        eStatusType_Accept,
        eStatusType_Decline
    };

    enum ETransitionType : uint32_t
    {
        eTransitionType_Undefined,
        eTransitionType_Quit = 2,
        eTransitionType_Dialog = 5,
        eTransitionType_Hide,
        eTransitionType_Abort,
        eTransitionType_SubMenu
    };

    class CHudPause : public CGameObject
    {
    public:
        xpointer<void> m_pVftable;
        SWA_INSERT_PADDING(0x2C);
        RCPtr<CProject> m_rcPause;
        RCPtr<CScene> m_rcBg;
        RCPtr<CScene> m_rcBg1;
        RCPtr<CScene> m_rcBg1_2;
        RCPtr<CScene> m_rcBg1Select;
        RCPtr<CScene> m_rcBg1Select_2;
        RCPtr<CScene> m_rcStatusTitle;
        RCPtr<CScene> m_rcFooterA;
        SWA_INSERT_PADDING(0x5C);
        be<EActionType> m_Action;
        be<EMenuType> m_Menu;
        be<EStatusType> m_Status;
        be<ETransitionType> m_Transition;
    };
}
