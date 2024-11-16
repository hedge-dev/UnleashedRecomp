#pragma once

#include "SWA.inl"
#include "Hedgehog/Universe/Engine/hhUpdateInfo.h"

namespace Hedgehog::Universe
{
    class IParallelJob
    {
    public:
        IParallelJob() {}
        IParallelJob(const swa_null_ctor&) {}

        virtual ~IParallelJob() = default;

        virtual void ExecuteParallelJob(const SUpdateInfo& in_rUpdateInfo) = 0;
    };
}
