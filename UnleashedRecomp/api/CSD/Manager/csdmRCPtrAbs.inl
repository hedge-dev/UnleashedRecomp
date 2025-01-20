namespace Chao::CSD
{
    inline RCPtrAbs::RCPtrAbs()
    {
        AttachAbs(nullptr);
    }

    inline RCPtrAbs::RCPtrAbs(void* in_pMemory)
    {
        AttachAbs(in_pMemory);
    }

    inline RCPtrAbs::RCPtrAbs(const RCPtrAbs& in_rOther)
    {
        SetAbs(in_rOther);
    }

    inline RCPtrAbs::RCPtrAbs(RCPtrAbs&& in_rPtr) : m_pObject(in_rPtr.m_pObject)
    {
        in_rPtr.m_pObject = nullptr;
    }

    inline RCPtrAbs::RCObject* RCPtrAbs::CreateRCObject()
    {
        return GuestToHostFunction<RCPtrAbs::RCObject*>(m_pVftable->m_fpCreateRCObject, this);
    }

    inline void RCPtrAbs::AttachAbs(void* in_pMemory)
    {
        GuestToHostFunction<void>(sub_830BA298, this, in_pMemory);
    }

    inline void RCPtrAbs::SetAbs(const RCPtrAbs& in_rPtr)
    {
        GuestToHostFunction<void>(sub_830BA328, this, in_rPtr);
    }

    inline void* RCPtrAbs::operator*() const
    {
        return GetAbs();
    }

    inline void* RCPtrAbs::operator->() const
    {
        return GetAbs();
    }

    inline RCPtrAbs& RCPtrAbs::operator=(const RCPtrAbs& in_rPtr)
    {
        SetAbs(in_rPtr);
        return *this;
    }

    inline RCPtrAbs::operator bool() const
    {
        return m_pObject != nullptr;
    }
}
