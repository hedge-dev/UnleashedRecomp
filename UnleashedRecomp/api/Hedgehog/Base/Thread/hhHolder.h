#pragma once

#include "Hedgehog/Base/Thread/hhHolderBase.h"

#define HH_BASE_HOLDER(x) \
    Hedgehog::Base::THolder(x)

namespace Hedgehog::Base
{
    template<typename T, bool ForceSync = true>
    class THolder : public CHolderBase
    {
    public:
        THolder(T* in_pObject) : CHolderBase(in_pObject, ForceSync) {}

        T* get() const;
        T* operator->() const;
        T* operator*() const;
    };
}
