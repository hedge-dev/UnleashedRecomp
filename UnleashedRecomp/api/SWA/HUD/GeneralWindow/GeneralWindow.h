#pragma once

#include <SWA.inl>

namespace SWA
{
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
    };
}
