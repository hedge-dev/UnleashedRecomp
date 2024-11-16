#pragma once

#include "SWA.inl"

namespace Hedgehog::Universe
{
    struct SUpdateInfo
    {
        be<float> DeltaTime;
        be<uint32_t> Frame;
        Base::CSharedString Category;
    };
}
