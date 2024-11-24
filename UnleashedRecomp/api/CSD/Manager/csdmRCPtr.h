#pragma once

#include "CSD/Manager/csdmRCPtrAbs.h"

namespace Chao::CSD
{
    template<typename T>
    class RCPtr : RCPtrAbs
    {
    public:
        class RCObjectImp;

        RCPtr() : RCPtrAbs() {}
        RCPtr(T* in_pMemory) : RCPtrAbs(in_pMemory) {}
        RCPtr(const RCPtr& in_rOther) : RCPtrAbs(in_rOther) {}
        RCPtr(RCPtr&& in_rOther) : RCPtrAbs(std::move(in_rOther)) {}

        void Attach(T* in_pObject);

        T* Get() const;
        void Set(const RCPtr& in_rOther);

        T* operator*() const;
        T* operator->() const;

        RCPtr& operator=(const RCPtr& in_rOther);

        operator bool() const;
    };
}

#include "CSD/Manager/csdmRCPtr.inl"
