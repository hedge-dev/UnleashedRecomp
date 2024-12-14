#pragma once

#include "heap.h"
#include "memory.h"

#define OBJECT_SIGNATURE           (DWORD)'XBOX'
#define GUEST_INVALID_HANDLE_VALUE 0xFFFFFFFF

struct KernelObject
{
    virtual ~KernelObject() 
    {
        ;
    }
};

template<typename T, typename... Args>
inline T* CreateKernelObject(Args&&... args)
{
    static_assert(std::is_base_of_v<KernelObject, T>);
    return g_userHeap.AllocPhysical<T>(std::forward<Args>(args)...);
}

template<typename T = KernelObject>
inline T* GetKernelObject(uint32_t handle)
{
    assert(handle != GUEST_INVALID_HANDLE_VALUE);
    return reinterpret_cast<T*>(g_memory.Translate(handle));
}

uint32_t GetKernelHandle(KernelObject* obj);

void DestroyKernelObject(KernelObject* obj);
void DestroyKernelObject(uint32_t handle);

bool IsKernelObject(uint32_t handle);
bool IsKernelObject(void* obj);

bool IsInvalidKernelObject(void* obj);

template<typename T = void>
inline T* GetInvalidKernelObject()
{
    return reinterpret_cast<T*>(g_memory.Translate(GUEST_INVALID_HANDLE_VALUE));
}

extern Mutex g_kernelLock;

template<typename T>
inline T* QueryKernelObject(XDISPATCHER_HEADER& header)
{
    std::lock_guard guard{ g_kernelLock };
    if (header.WaitListHead.Flink != OBJECT_SIGNATURE)
    {
        header.WaitListHead.Flink = OBJECT_SIGNATURE;
        auto* obj = CreateKernelObject<T>(reinterpret_cast<typename T::guest_type*>(&header));
        header.WaitListHead.Blink = g_memory.MapVirtual(obj);

        return obj;
    }

    return static_cast<T*>(g_memory.Translate(header.WaitListHead.Blink.get()));
}

// Get object without initialisation
template<typename T>
inline T* TryQueryKernelObject(XDISPATCHER_HEADER& header)
{
    if (header.WaitListHead.Flink != OBJECT_SIGNATURE)
        return nullptr;

    return static_cast<T*>(g_memory.Translate(header.WaitListHead.Blink.get()));
}
