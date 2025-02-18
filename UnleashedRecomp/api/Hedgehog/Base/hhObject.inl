namespace Hedgehog::Base
{
    inline CObject::CObject()
    {
    }

    inline CObject::CObject(const swa_null_ctor&)
    {
    }

    inline void* CObject::operator new(const size_t in_Size)
    {
        return __HH_ALLOC(in_Size);
    }

    // inline void* CObject::operator new(const size_t in_Size, const size_t in_Align)
    // {
    //     return __HH_ALLOCALIGN(in_Size, in_Align);
    // }

    inline void CObject::operator delete(void* in_pMem)
    {
        return __HH_FREE(in_pMem);
    }

    inline void* CObject::operator new(const size_t in_Size, void* in_pObj)
    {
        return in_pObj;
    }

    inline void* CObject::operator new(const size_t in_Size, const size_t in_Align, void* in_pObj)
    {
        return in_pObj;
    }

    inline void CObject::operator delete(void* in_pMem, void* in_pObj)
    {
    }
}
