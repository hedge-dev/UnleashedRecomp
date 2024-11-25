#pragma once

#include "SWA.inl"
#include "Hedgehog/MirageCore/Renderable/hhRenderable.h"

namespace SWA::Sequence::Utility
{
    class CPlayMovieWrapper
    {
    public:
        class CRender : public Hedgehog::Mirage::CRenderable
        {
        public:
            be<uint32_t> m_ScreenWidth;
            be<uint32_t> m_ScreenHeight;
            be<float> m_MovieWidth;
            be<float> m_MovieHeight;
            SWA_INSERT_PADDING(0x74);
            be<float> m_TopLeftX;
            be<float> m_TopLeftY;
            SWA_INSERT_PADDING(0x0C);
            be<float> m_TopRightX;
            be<float> m_TopRightY;
            SWA_INSERT_PADDING(0x0C);
            be<float> m_BottomRightX;
            be<float> m_BottomRightY;
            SWA_INSERT_PADDING(0x0C);
            be<float> m_BottomLeftX;
            be<float> m_BottomLeftY;
            SWA_INSERT_PADDING(0xD4);
            bool m_MaintainAspectRatio;
            SWA_INSERT_PADDING(0x18);
            be<float> m_TimeElapsed;
        };

        SWA_INSERT_PADDING(0x18);
        xpointer<CRender> m_pRender;
    };
}
