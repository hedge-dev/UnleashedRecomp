#pragma once

#include <SWA.inl>

namespace SWA::Inspire
{
    class CMovieOverlayInfo
    {
    public:
        Hedgehog::Base::CSharedString m_MovieName;
        be<float> m_StartTime;
        be<float> m_FadeInStartTime;
        be<float> m_FadeInEndTime;
        be<float> m_FadeOutStartTime;
        be<float> m_FadeOutEndTime;
    };
}
