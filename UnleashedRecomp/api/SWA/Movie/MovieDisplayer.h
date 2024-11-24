#pragma once

#include <SWA.inl>

namespace SWA
{
    class CMovieDisplayer : public Hedgehog::Universe::CUpdateUnit, public Hedgehog::Mirage::CRenderable
    {
    public:
        SWA_INSERT_PADDING(0x04);
        be<uint32_t> m_MovieWidth;
        be<uint32_t> m_MovieHeight;
    };
}
