#pragma once

#include <SWA.inl>

namespace SWA
{
    class CMovieDisplayer : public Hedgehog::Universe::CUpdateUnit, public Hedgehog::Mirage::CRenderable
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

        SWA_INSERT_PADDING(0x04);
        be<uint32_t> m_MovieWidth;
        be<uint32_t> m_MovieHeight;
        SWA_INSERT_PADDING(0x74);
        SVertexData m_TopLeft;
        SVertexData m_TopRight;
        SVertexData m_BottomRight;
        SVertexData m_BottomLeft;
        SWA_INSERT_PADDING(0xF0);
    };
}
