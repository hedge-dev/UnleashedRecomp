#pragma once

#include <SWA.inl>

namespace SWA
{
    class CObjHintRing
    {
    public:
        SWA_INSERT_PADDING(0x160);
        be<uint32_t> m_AnimationID;
        bool m_Field164;
    };
}
