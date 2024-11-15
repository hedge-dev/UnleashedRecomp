namespace Chao::CSD
{
    inline void RCPtrAbs::RCObject::Release()
    {
        GuestToHostFunction<void>(0x830BA068, this);
    }

    inline RCPtrAbs::~RCPtrAbs()
    {
        RCObject* pObj = m_pObject;
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
}
