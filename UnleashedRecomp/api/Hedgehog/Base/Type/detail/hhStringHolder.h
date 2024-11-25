#pragma once

#include <SWA.inl>

namespace Hedgehog::Base
{
    struct SStringHolder
    {
        union
        {
            struct
            {
                be<uint16_t> Length;
                be<uint16_t> RefCount;
            };

            be<uint32_t> RefCountAndLength;
        };

        char aStr[1u];

        static SStringHolder* GetHolder(const char* in_pStr);

        static size_t GetMemorySize(const size_t in_Length);
        static size_t GetMemorySizeAligned(const size_t in_Length);

        static SStringHolder* Make(const char* in_pStr, const size_t in_Length);

        static SStringHolder* Concat(
            const char* in_pStrA, const size_t in_LengthA,
            const char* in_pStrB, const size_t in_LengthB);

        void AddRef();
        void Release();

        bool IsUnique() const;

        bool TryInplaceAssign(const char* in_pStr, const size_t in_Length);
        bool TryInplaceAppend(const char* in_pStr, const size_t in_Length);
        bool TryInplacePrepend(const char* in_pStr, const size_t in_Length);
    };
}

#include <Hedgehog/Base/Type/detail/hhStringHolder.inl>
