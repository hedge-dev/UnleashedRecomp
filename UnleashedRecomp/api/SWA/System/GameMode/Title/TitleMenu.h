#pragma once

#include <SWA.inl>

namespace SWA
{
    class CTitleMenu
    {
    public:
        SWA_INSERT_PADDING(0x44);
        be<uint32_t> m_CursorIndex;
    };
}
