#pragma once

#include <SWA.inl>

namespace Hedgehog::Base
{
    struct SStringHolder
    {
        be<uint32_t> RefCount;
        char aStr[];

        static SStringHolder* GetHolder(const char* in_pStr);
        static SStringHolder* Make(const char* in_pStr);

        void AddRef();
        void Release();

        bool IsUnique() const;
    };
}

#include <Hedgehog/Base/Type/detail/hhStringHolder.inl>
