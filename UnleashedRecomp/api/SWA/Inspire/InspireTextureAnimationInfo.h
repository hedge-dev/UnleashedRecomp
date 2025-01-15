#pragma once

#include <SWA.inl>

namespace SWA::Inspire
{
    class CTextureAnimationInfo
    {
    public:
        SWA_INSERT_PADDING(0x10);
        Hedgehog::Base::CSharedString m_MovieTex;
        Hedgehog::Base::CSharedString m_MovieSfd;
        SWA_INSERT_PADDING(0x08);
        be<float> m_Prepare;
        be<float> m_Start;
        be<float> m_End;
        be<float> m_Width;
        be<float> m_Height;
    };
}
