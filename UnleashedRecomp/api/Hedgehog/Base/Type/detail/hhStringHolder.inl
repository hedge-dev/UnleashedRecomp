namespace Hedgehog::Base
{
    inline SStringHolder* SStringHolder::GetHolder(const char* in_pStr)
    {
        return (SStringHolder*)((size_t)in_pStr - sizeof(RefCountAndLength));
    }

    inline size_t SStringHolder::GetMemorySize(const size_t in_Length)
    {
        return sizeof(RefCountAndLength) + in_Length;
    }

    inline size_t SStringHolder::GetMemorySizeAligned(const size_t in_Length)
    {
        return (GetMemorySize(in_Length) + 0x10) & ~0x0F;
    }

    inline SStringHolder* SStringHolder::Make(const char* in_pStr, const size_t in_Length)
    {
        const size_t memSize = GetMemorySize(in_Length);
        const size_t memSizeAligned = GetMemorySizeAligned(in_Length);

        auto pHolder = (SStringHolder*)__HH_ALLOC(memSizeAligned);
        pHolder->RefCount = 1;
        pHolder->Length = (uint16_t)in_Length;

        if (in_pStr)
            memcpy(pHolder->aStr, in_pStr, in_Length);

        memset(&pHolder->aStr[in_Length], 0, memSizeAligned - memSize);

        return pHolder;
    }

    inline SStringHolder* SStringHolder::Concat(const char* in_pStrA, const size_t in_LengthA, const char* in_pStrB, const size_t in_LengthB)
    {
        SStringHolder* pHolder = Make(nullptr, in_LengthA + in_LengthB);

        memcpy(pHolder->aStr, in_pStrA, in_LengthA);
        memcpy(&pHolder->aStr[in_LengthA], in_pStrB, in_LengthB);

        return pHolder;
    }

    inline void SStringHolder::AddRef()
    {
        uint32_t originalValue, incrementedValue;
        do
        {
            originalValue = RefCountAndLength.value;
            incrementedValue = std::byteswap(std::byteswap(originalValue) + 1);
        } while (InterlockedCompareExchange(reinterpret_cast<LONG*>(&RefCountAndLength), incrementedValue, originalValue) != originalValue);
    }

    inline void SStringHolder::Release()
    {
        uint32_t originalValue, decrementedValue;
        do
        {
            originalValue = RefCountAndLength.value;
            decrementedValue = std::byteswap(std::byteswap(originalValue) - 1);
        } while (InterlockedCompareExchange(reinterpret_cast<LONG*>(&RefCountAndLength), decrementedValue, originalValue) != originalValue);

        if (RefCountAndLength == 0)
            __HH_FREE(this);
    }

    inline bool SStringHolder::IsUnique() const
    {
        return RefCount == 1;
    }

    inline bool SStringHolder::TryInplaceAssign(const char* in_pStr, const size_t in_Length)
    {
        if (!IsUnique())
            return false;

        const size_t memSizeAligned = GetMemorySizeAligned(in_Length);

        if (memSizeAligned > GetMemorySizeAligned(Length))
            return false;

        if (in_pStr)
            memcpy(aStr, in_pStr, in_Length);

        memset(&aStr[in_Length], 0, memSizeAligned - GetMemorySize(in_Length));

        Length = (uint16_t)in_Length;

        return true;
    }

    inline bool SStringHolder::TryInplaceAppend(const char* in_pStr, const size_t in_Length)
    {
        if (!IsUnique())
            return false;

        const size_t memSizeAligned = GetMemorySizeAligned(Length + in_Length);

        if (memSizeAligned > GetMemorySizeAligned(Length))
            return false;

        if (in_pStr)
            memcpy(&aStr[Length], in_pStr, in_Length);

        memset(&aStr[Length + in_Length], 0, memSizeAligned - GetMemorySize(Length + in_Length));

        Length = (uint16_t)(Length + in_Length);

        return true;
    }

    inline bool SStringHolder::TryInplacePrepend(const char* in_pStr, const size_t in_Length)
    {
        if (!IsUnique())
            return false;

        const size_t memSizeAligned = GetMemorySizeAligned(in_Length + Length);

        if (memSizeAligned > GetMemorySizeAligned(Length))
            return false;

        memmove(&aStr[in_Length], aStr, Length);

        if (in_pStr)
            memcpy(aStr, in_pStr, in_Length);

        memset(&aStr[in_Length + Length], 0, memSizeAligned - GetMemorySize(in_Length + Length));

        Length = (uint16_t)(in_Length + Length);

        return true;
    }
}
