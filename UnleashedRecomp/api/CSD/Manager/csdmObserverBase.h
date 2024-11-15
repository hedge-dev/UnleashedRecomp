#pragma once

#include "SWA.inl"

namespace Chao::CSD
{
    template<typename TObservee>
    class CObserverBase
    {
    public:
        SWA_INSERT_PADDING(0x0C);

        virtual ~CObserverBase() = default;
    };
}
