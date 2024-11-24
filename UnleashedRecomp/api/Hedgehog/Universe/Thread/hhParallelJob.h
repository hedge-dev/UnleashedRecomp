#pragma once

#include "SWA.inl"
#include "Hedgehog/Universe/Engine/hhUpdateInfo.h"

namespace Hedgehog::Universe
{
    class IParallelJob
    {
    public:
        xpointer<void> m_pVftable;

        IParallelJob() {}
        IParallelJob(const swa_null_ctor&) {}

        // TODO: implement virtual functions.
        // virtual ~IParallelJob() = default;
        // 
        // virtual void ExecuteParallelJob(const SUpdateInfo& in_rUpdateInfo) = 0;
    };
}
