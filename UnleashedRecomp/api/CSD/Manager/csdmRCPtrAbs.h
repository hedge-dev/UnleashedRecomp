#pragma once

#include "SWA.inl"

namespace Chao::CSD
{
    class RCPtrAbs
    {
    public:
        class RCObject;

        xpointer<RCObject> m_pObject;

        RCPtrAbs();
        RCPtrAbs(void* in_pMemory);
        RCPtrAbs(const RCPtrAbs& in_rOther);
        RCPtrAbs(RCPtrAbs&& in_rPtr);

        virtual ~RCPtrAbs();
        virtual RCObject* CreateRCObject() = 0;

        void AttachAbs(void* in_pMemory);
        void* GetAbs() const;
        void SetAbs(const RCPtrAbs& in_rPtr);

        void* operator*() const;
        void* operator->() const;

        RCPtrAbs& operator=(const RCPtrAbs& in_rPtr);

        operator bool() const;
    };
}
