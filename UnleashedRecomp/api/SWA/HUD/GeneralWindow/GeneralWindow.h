#pragma once

#include <SWA.inl>

namespace SWA
{
    enum EWindowStatus : uint32_t
    {
        eWindowStatus_Closed,
        eWindowStatus_OpeningMessage = 2,
        eWindowStatus_DisplayingMessage,
        eWindowStatus_OpeningControls,
        eWindowStatus_DisplayingControls
    };

    class CGeneralWindow
    {
    public:
        SWA_INSERT_PADDING(0xD0);
        Chao::CSD::RCPtr<Chao::CSD::CProject> m_rcGeneral;
        Chao::CSD::RCPtr<Chao::CSD::CScene> m_rcBg;
        Chao::CSD::RCPtr<Chao::CSD::CScene> m_rcWindow;
        Chao::CSD::RCPtr<Chao::CSD::CScene> m_rcWindow_2;
        Chao::CSD::RCPtr<Chao::CSD::CScene> m_rcWindowSelect;
        Chao::CSD::RCPtr<Chao::CSD::CScene> m_rcFooter;
        SWA_INSERT_PADDING(0x58);
        be<EWindowStatus> m_Status;
        be<uint32_t> m_CursorIndex;
        SWA_INSERT_PADDING(0x04);
        be<uint32_t> m_SelectedIndex;
    };

    SWA_ASSERT_OFFSETOF(CGeneralWindow, m_rcGeneral, 0xD0);
    SWA_ASSERT_OFFSETOF(CGeneralWindow, m_rcBg, 0xD8);
    SWA_ASSERT_OFFSETOF(CGeneralWindow, m_rcWindow, 0xE0);
    SWA_ASSERT_OFFSETOF(CGeneralWindow, m_rcWindow_2, 0xE8);
    SWA_ASSERT_OFFSETOF(CGeneralWindow, m_rcWindowSelect, 0xF0);
    SWA_ASSERT_OFFSETOF(CGeneralWindow, m_rcFooter, 0xF8);
    SWA_ASSERT_OFFSETOF(CGeneralWindow, m_Status, 0x158);
    SWA_ASSERT_OFFSETOF(CGeneralWindow, m_CursorIndex, 0x15C);
    SWA_ASSERT_OFFSETOF(CGeneralWindow, m_SelectedIndex, 0x164);
}
