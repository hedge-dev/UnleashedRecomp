namespace Hedgehog::Base
{
    inline SStringHolder* SStringHolder::GetHolder(const char* in_pStr)
    {
        return (SStringHolder*)((size_t)in_pStr - sizeof(RefCount));
    }

    inline SStringHolder* SStringHolder::Make(const char* in_pStr)
    {
        auto pHolder = (SStringHolder*)__HH_ALLOC(sizeof(RefCount) + strlen(in_pStr) + 1);
        pHolder->RefCount = 1;
        strcpy(pHolder->aStr, in_pStr);
        return pHolder;
    }

    inline void SStringHolder::AddRef()
    {
        std::atomic_ref refCount(RefCount.value);

        be<uint32_t> original, incremented;
        do
        {
            original = RefCount;
            incremented = original + 1;
        } while (!refCount.compare_exchange_weak(original.value, incremented.value));
    }

    inline void SStringHolder::Release()
    {
        std::atomic_ref refCount(RefCount.value);

        be<uint32_t> original, decremented;
        do
        {
            original = RefCount;
            decremented = original - 1;
        } while (!refCount.compare_exchange_weak(original.value, decremented.value));

        if (decremented == 0)
            __HH_FREE(this);
    }

    inline bool SStringHolder::IsUnique() const
    {
        return RefCount == 1;
    }
}
