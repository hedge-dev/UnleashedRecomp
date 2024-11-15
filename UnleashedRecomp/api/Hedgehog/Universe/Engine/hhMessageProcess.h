#pragma once

#include "SWA.inl"
#include "Hedgehog/Base/hhObject.h"

namespace Hedgehog::Universe
{
    class IMessageProcess : public Base::CObject
    {
    public:
        IMessageProcess() {}
        IMessageProcess(const swa_null_ctor& nil) : CObject(nil) {}
    };
}
