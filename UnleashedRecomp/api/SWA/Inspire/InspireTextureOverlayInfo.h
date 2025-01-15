#pragma once

#include <SWA.inl>

namespace SWA::Inspire
{
    class CTextureOverlayInfo
    {
    public:
        Hedgehog::Base::CSharedString m_Picture;
        be<uint32_t> m_Start;
        be<uint32_t> m_End;
    };
}
