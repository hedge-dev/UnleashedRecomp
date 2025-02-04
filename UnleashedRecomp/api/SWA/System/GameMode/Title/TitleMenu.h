#pragma once

#include <SWA.inl>

namespace SWA
{
    class CTitleMenu : public CMenuWindowBase
    {
    public:
        SWA_INSERT_PADDING(0x28);
        be<uint32_t> m_Field38;
        bool m_Field3C; // Seems to be related to exit transition.
        SWA_INSERT_PADDING(0x04);
        be<uint32_t> m_CursorIndex;
        SWA_INSERT_PADDING(0x0C);
        bool m_Field54; // Seems to be related to exit transition.
        SWA_INSERT_PADDING(0x0B);
        be<float> m_Field60;
        SWA_INSERT_PADDING(0x34);
        bool m_Field98;
        bool m_IsDeleteCheckMessageOpen;
        bool m_Field9A; // Seems to be related to cursor selection.
        SWA_INSERT_PADDING(0x04);
        bool m_Field9F;
        SWA_INSERT_PADDING(0x02);
        bool m_IsDLCInfoMessageOpen;
    };

    SWA_ASSERT_OFFSETOF(CTitleMenu, m_Field38, 0x38);
    SWA_ASSERT_OFFSETOF(CTitleMenu, m_Field3C, 0x3C);
    SWA_ASSERT_OFFSETOF(CTitleMenu, m_CursorIndex, 0x44);
    SWA_ASSERT_OFFSETOF(CTitleMenu, m_Field54, 0x54);
    SWA_ASSERT_OFFSETOF(CTitleMenu, m_Field60, 0x60);
    SWA_ASSERT_OFFSETOF(CTitleMenu, m_Field98, 0x98);
    SWA_ASSERT_OFFSETOF(CTitleMenu, m_IsDeleteCheckMessageOpen, 0x99);
    SWA_ASSERT_OFFSETOF(CTitleMenu, m_Field9A, 0x9A);
    SWA_ASSERT_OFFSETOF(CTitleMenu, m_Field9F, 0x9F);
    SWA_ASSERT_OFFSETOF(CTitleMenu, m_IsDLCInfoMessageOpen, 0xA2);
}
