#pragma once

#include <SWA.inl>

namespace SWA
{
    class CWorldMapCursor : public CMenuWindowBase
    {
    public:
        SWA_INSERT_PADDING(0x24);
        be<float> m_LeftStickVertical;
        be<float> m_LeftStickHorizontal;
        bool m_IsCursorMoving;
        SWA_INSERT_PADDING(0x07);
        be<float> m_CursorY;
        be<float> m_CursorX;
    };

    SWA_ASSERT_OFFSETOF(CWorldMapCursor, m_LeftStickVertical, 0x34);
    SWA_ASSERT_OFFSETOF(CWorldMapCursor, m_LeftStickHorizontal, 0x38);
    SWA_ASSERT_OFFSETOF(CWorldMapCursor, m_IsCursorMoving, 0x3C);
    SWA_ASSERT_OFFSETOF(CWorldMapCursor, m_CursorY, 0x44);
    SWA_ASSERT_OFFSETOF(CWorldMapCursor, m_CursorX, 0x48);
}
