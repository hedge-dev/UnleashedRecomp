#pragma once

#include "CSD/Manager/csdmRCPtr.h"

namespace Chao::CSD
{
    template<typename T>
    class RCPtr<T>::RCObjectImp : public RCObject {};
}
