#pragma once

#include "SWA.inl"
#include "Hedgehog/MirageCore/Renderable/hhRenderable.h"

namespace SWA::Sequence::Utility
{
    class CPlayMovieWrapper
    {
        class CRender : public Hedgehog::Mirage::CRenderable
        {
            SWA_INSERT_PADDING(0x08);
            be<uint32_t> m_ScreenWidth;
            be<uint32_t> m_ScreenHeight;
            be<float> m_MovieWidth;
            be<float> m_MovieHeight;
        };

        SWA_INSERT_PADDING(0x18);
        CRender* m_pRender;
    };
}
