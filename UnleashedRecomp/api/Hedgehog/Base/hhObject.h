#pragma once

#include "SWA.inl"

namespace Hedgehog::Base
{
    class CObject
    {
    public:
        CObject();
        CObject(const swa_null_ctor&);

        void* operator new(const size_t in_Size);
        void* operator new(const size_t in_Size, const size_t in_Align);

        void operator delete(void* in_pMem);

        void* operator new(const size_t in_Size, void* in_pObj);
        void* operator new(const size_t in_Size, const size_t in_Align, void* in_pObj);

        void operator delete(void* in_pMem, void* in_pObj);
    };
}
