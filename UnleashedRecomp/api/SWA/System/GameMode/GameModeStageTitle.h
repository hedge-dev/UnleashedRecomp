#pragma once

#include <SWA.inl>

namespace SWA
{
    class CGameModeStageTitle : public CGameModeStage
    {
    public:
        SWA_INSERT_PADDING(0x0E);
        bool m_IsPlayingAdvertiseMovie;
        be<float> m_AdvertiseMovieWaitTime;
    };
}
