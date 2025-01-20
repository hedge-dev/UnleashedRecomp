namespace Chao::CSD
{
    inline RCPtrAbs::RCObject::~RCObject()
    {
        GuestToHostFunction<void>(m_pVftable->m_fpDtor, this);
    }

    inline RCPtrAbs::~RCPtrAbs()
    {
        RCPtrAbs::RCObject* pObj = m_pObject;

        m_pObject = nullptr;

        if (pObj)
            pObj->Release();
    }

    inline void* RCPtrAbs::GetAbs() const
    {
        if (!m_pObject)
            return nullptr;

        return m_pObject->m_pMemory;
    }

    inline void RCPtrAbs::RCObject::Deallocate(void* in_pMemory)
    {
        GuestToHostFunction<void>(m_pVftable->m_fpDeallocate, this, in_pMemory);
    }

    inline void RCPtrAbs::RCObject::Release()
    {
        GuestToHostFunction<void>(sub_830BA068, this);
    }
}
