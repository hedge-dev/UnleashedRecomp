#pragma once

#include "SWA.inl"

namespace Chao::CSD
{
    template<typename TObserver, typename TObservee>
    class SubjectBase
    {
    public:
        SWA_INSERT_PADDING(0x0C);

        virtual ~SubjectBase() = default;
        virtual TObservee* GetObservee() const { return nullptr; }
    };
}
