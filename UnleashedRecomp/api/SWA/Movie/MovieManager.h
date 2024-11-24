#pragma once

#include <SWA.inl>

namespace SWA
{
    class CMovieManager // : Hedgehog::Base::TSingleton<SWA::CMovieManager, 0>
    {
    public:
        static CMovieManager* GetInstance();

        xpointer<void> m_pVftable;
        SWA_INSERT_PADDING(0x330);
    };
}

#include "SWA/Movie/MovieManager.inl"
