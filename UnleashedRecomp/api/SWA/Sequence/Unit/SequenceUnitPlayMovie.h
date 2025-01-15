#pragma once

#include <SWA.inl>

namespace SWA::Sequence::Unit
{
    class CPlayMovieUnit : public CUnitBase
    {
    public:
        Hedgehog::Base::CSharedString m_SceneName;
    };
}
