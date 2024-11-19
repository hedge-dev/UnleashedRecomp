#pragma once

#include "SWA.inl"
#include "Hedgehog/Universe/Engine/hhUpdateInfo.h"

namespace Hedgehog::Universe
{
    class IParallelJob
    {
    public:
        SWA_INSERT_PADDING(0x04); // vftable ptr

        IParallelJob() {}
        IParallelJob(const swa_null_ctor&) {}

        // TODO: implement virtual functions.
        // virtual ~IParallelJob() = default;
        // 
        // virtual void ExecuteParallelJob(const SUpdateInfo& in_rUpdateInfo) = 0;
    };
}
