#pragma once

#include "SWA.inl"
#include "Hedgehog/Base/Thread/hhHolder.h"

namespace Hedgehog::Base
{
    template<typename T, bool ForceSync = false>
    class TSynchronizedPtr
    {
        xpointer<T> m_pObject;

    public:
        TSynchronizedPtr(T* in_pObject);
        TSynchronizedPtr();
        TSynchronizedPtr(const TSynchronizedPtr& in_rOther);
        TSynchronizedPtr(TSynchronizedPtr&& io_rOther);

        TSynchronizedPtr<T, ForceSync>& operator=(T* const in_pObject);

        THolder<T, ForceSync> get() const;
        THolder<T, ForceSync> operator->() const;
        THolder<T, ForceSync> operator*() const;

        explicit operator bool() const;
    };
}
