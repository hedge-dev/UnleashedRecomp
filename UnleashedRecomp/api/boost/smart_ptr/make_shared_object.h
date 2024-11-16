#pragma once

#include "boost/smart_ptr/shared_ptr.h"

namespace boost
{
    template<typename T, typename... TArgs>
    shared_ptr<T> make_shared(TArgs&&... args)
    {
        return shared_ptr<T>(new T(std::forward<TArgs>(args)...));
    }
}
