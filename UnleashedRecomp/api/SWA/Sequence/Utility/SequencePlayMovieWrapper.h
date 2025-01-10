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
            Vertex m_TopLeft;
            Vertex m_TopRight;
            Vertex m_BottomRight;
            Vertex m_BottomLeft;
            bool m_MaintainAspectRatio;
            SWA_INSERT_PADDING(0x18);
            be<float> m_TimeElapsed;
        };

        SWA_INSERT_PADDING(0x18);
        xpointer<CRender> m_pRender;
    };
}
