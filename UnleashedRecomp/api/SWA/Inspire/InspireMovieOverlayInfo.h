#pragma once

#include <SWA.inl>

namespace SWA::Inspire
{
    class CMovieOverlayInfo
    {
    public:
        Hedgehog::Base::CSharedString m_MovieName;
        be<float> m_Prepare;
        be<float> m_InStart;
        be<float> m_InEnd;
        be<float> m_OutStart;
        be<float> m_OutEnd;
    };
}
