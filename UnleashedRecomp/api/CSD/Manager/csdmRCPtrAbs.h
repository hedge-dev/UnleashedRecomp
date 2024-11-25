#pragma once

#include "SWA.inl"

namespace Chao::CSD
{
    class RCPtrAbs
    {
    public:
        class RCObject;

        struct Vftable
        {
            be<uint32_t> m_fpDtor;
            be<uint32_t> m_fpCreateRCObject;
        };

        xpointer<Vftable> m_pVftable;
        xpointer<RCObject> m_pObject;

        RCPtrAbs();
        RCPtrAbs(void* in_pMemory);
        RCPtrAbs(const RCPtrAbs& in_rOther);
        RCPtrAbs(RCPtrAbs&& in_rPtr);

        ~RCPtrAbs();
        RCObject* CreateRCObject();

        void AttachAbs(void* in_pMemory);
        void* GetAbs() const;
        void SetAbs(const RCPtrAbs& in_rPtr);

        void* operator*() const;
        void* operator->() const;

        RCPtrAbs& operator=(const RCPtrAbs& in_rPtr);

        operator bool() const;
    };
}

#include "CSD/Manager/csdmRCPtrAbs.inl"
