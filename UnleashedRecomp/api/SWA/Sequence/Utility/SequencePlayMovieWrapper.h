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
            struct SVertexData
            {
                be<float> X;
                be<float> Y;
                be<float> Z;
                be<float> U;
                be<float> V;
            };

            be<uint32_t> m_ScreenWidth;
            be<uint32_t> m_ScreenHeight;
            be<float> m_MovieWidth;
            be<float> m_MovieHeight;
            SWA_INSERT_PADDING(0x74);
            SVertexData m_TopLeft;
            SVertexData m_TopRight;
            SVertexData m_BottomRight;
            SVertexData m_BottomLeft;
            bool m_Field1A4;
            SWA_INSERT_PADDING(0x18);
            be<float> m_TimeElapsed;
        };

        xpointer<void> m_pVftable;
        Hedgehog::Base::CSharedString m_SceneName;
        SWA_INSERT_PADDING(0x10);
        xpointer<CRender> m_pRender;
        SWA_INSERT_PADDING(0x04);
        xpointer<Hedgehog::Base::CSharedString> m_pResourceName;
    };
}
