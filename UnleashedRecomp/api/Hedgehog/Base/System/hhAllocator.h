#pragma once

#include "SWA.inl"

inline static void* __HH_ALLOC(const uint32_t in_Size)
{
    return GuestToHostFunction<void*>(0x82DFA0B0, in_Size, nullptr, 0, 0);
}

inline static void __HH_FREE(const void* in_pData)
{
    GuestToHostFunction<void>(0x82DF9E50, in_pData);
}

namespace Hedgehog::Base
{
    template<class T>
    class TAllocator
    {
    public:
        using value_type = T;

        TAllocator() noexcept {}
        template<class U> TAllocator(TAllocator<U> const&) noexcept {}

        value_type* allocate(std::size_t n)
        {
            return reinterpret_cast<value_type*>(__HH_ALLOC(n * sizeof(value_type)));
        }

        void deallocate(value_type* p, std::size_t) noexcept
        {
            __HH_FREE(p);
        }
    };
}
