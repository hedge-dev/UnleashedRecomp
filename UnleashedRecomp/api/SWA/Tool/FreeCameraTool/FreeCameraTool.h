#pragma once

#include <SWA.inl>

namespace SWA
{
    class CFreeCameraTool : public CGameObject
    {
    public:
        SWA_INSERT_PADDING(0x04);
        xpointer<CFreeCamera> m_pFreeCamera;
    };

    SWA_ASSERT_OFFSETOF(CFreeCameraTool, m_pFreeCamera, 0xC0);
}
