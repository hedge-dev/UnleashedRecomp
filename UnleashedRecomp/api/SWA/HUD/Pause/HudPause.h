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
        SWA_INSERT_PADDING(0x30);
        RCPtr<CProject> m_rcPause;
        RCPtr<CScene> m_rcBg;
        RCPtr<CScene> m_rcBg1;
        RCPtr<CScene> m_rcBg1_2;
        RCPtr<CScene> m_rcBg1Select;
        RCPtr<CScene> m_rcBg1Select_2;
        RCPtr<CScene> m_rcStatusTitle;
        RCPtr<CScene> m_rcFooterA;
        SWA_INSERT_PADDING(0x59);
        bool m_IsVisible;
        SWA_INSERT_PADDING(0x02);
        be<EActionType> m_Action;
        be<EMenuType> m_Menu;
        be<EStatusType> m_Status;
        be<ETransitionType> m_Transition;
        SWA_INSERT_PADDING(0x04);
        be<uint32_t> m_Submenu;
        SWA_INSERT_PADDING(0x18);
        bool m_IsShown;
    };

    SWA_ASSERT_OFFSETOF(CHudPause, m_rcPause, 0xEC);
    SWA_ASSERT_OFFSETOF(CHudPause, m_rcBg, 0xF4);
    SWA_ASSERT_OFFSETOF(CHudPause, m_rcBg1, 0xFC);
    SWA_ASSERT_OFFSETOF(CHudPause, m_rcBg1_2, 0x104);
    SWA_ASSERT_OFFSETOF(CHudPause, m_rcBg1Select, 0x10C);
    SWA_ASSERT_OFFSETOF(CHudPause, m_rcBg1Select_2, 0x114);
    SWA_ASSERT_OFFSETOF(CHudPause, m_rcStatusTitle, 0x11C);
    SWA_ASSERT_OFFSETOF(CHudPause, m_rcFooterA, 0x124);
    SWA_ASSERT_OFFSETOF(CHudPause, m_IsVisible, 0x185);
    SWA_ASSERT_OFFSETOF(CHudPause, m_Action, 0x188);
    SWA_ASSERT_OFFSETOF(CHudPause, m_Menu, 0x18C);
    SWA_ASSERT_OFFSETOF(CHudPause, m_Status, 0x190);
    SWA_ASSERT_OFFSETOF(CHudPause, m_Transition, 0x194);
    SWA_ASSERT_OFFSETOF(CHudPause, m_Submenu, 0x19C);
    SWA_ASSERT_OFFSETOF(CHudPause, m_IsShown, 0x1B8);
}
