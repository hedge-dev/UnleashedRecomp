#pragma once

#include <SWA.inl>

namespace SWA
{
    class CCameraController
    {
    public:
        SWA_INSERT_PADDING(0x64);
        be<float> m_FieldOfView;
        SWA_INSERT_PADDING(0x68);
    };

    SWA_ASSERT_OFFSETOF(CCameraController, m_FieldOfView, 0x64);
    SWA_ASSERT_SIZEOF(CCameraController, 0xD0);
}
