#pragma once

#include "ppc_context.h"
#include <kernel/memory.h>

// DO NOT use this type as anything other than a local variable.
// This includes returning. It'll cause memory to leak in the guest stack!

template<typename T, bool Init = true>
class guest_stack_var
{
private:
    uint32_t m_ptr = NULL;
    uint32_t m_oldStackPtr = NULL;

    void AllocGuestStackMemory()
    {
        auto ctx = GetPPCContext();
        m_oldStackPtr = ctx->r1.u32;
        m_ptr = (ctx->r1.u32 - sizeof(T)) & ~(std::max<uint32_t>(alignof(T), 8) - 1);
        ctx->r1.u32 = m_ptr;
    }

public:
    T* get()
    {
        return reinterpret_cast<T*>(g_memory.Translate(m_ptr));
    }

    const T* get() const
    {
        return reinterpret_cast<const T*>(g_memory.Translate(m_ptr));
    }

    template<typename... Args>
    guest_stack_var(Args&&... args)
    {
        AllocGuestStackMemory();

        if (Init)
            new (get()) T(std::forward<Args>(args)...);
    }

    guest_stack_var(const guest_stack_var<T>& other)
    {
        AllocGuestStackMemory();

        if (Init)
            new (get()) T(*other->get());
    }

    guest_stack_var(guest_stack_var<T>&& other)
    {
        AllocGuestStackMemory();

        if (Init)
            new (get()) T(std::move(*other->get()));
    }

    ~guest_stack_var()
    {
        get()->~T();

        auto ctx = GetPPCContext();
        // This assert will fail if the type was used as anything other than a local variable.
        assert(ctx->r1.u32 == m_ptr);
        ctx->r1.u32 = m_oldStackPtr;
    }

    void operator=(const guest_stack_var<T>& other)
    {
        if (this != &other)
            *get() = *other->get();
    }

    void operator=(guest_stack_var<T>&& other)
    {
        if (this != &other)
            *get() = std::move(*other->get());
    }

    void operator=(const T& other)
    {
        if (get() != &other)
            *get() = *other;
    }

    void operator=(T&& other)
    {
        if (get() != &other)
            *get() = std::move(*other);
    }

    operator const T* () const
    {
        return get();
    }

    operator T* ()
    {
        return get();
    }

    const T* operator->() const
    {
        return get();
    }

    T* operator->()
    {
        return get();
    }

    const T& operator*() const
    {
        return *get();
    }

    T& operator*()
    {
        return *get();
    }
};
