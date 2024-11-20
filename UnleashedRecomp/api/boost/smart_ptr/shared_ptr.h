#pragma once

#include <cstddef>
#include <utility>

namespace boost
{
    template<typename T>
    class shared_ptr
    {
    private:
        xpointer<T> m_pObject;
        xpointer<uint32_t> m_pRefCount;

        void release()
        {
            if (m_pRefCount && --(*m_pRefCount) == 0)
            {
                delete m_pObject;
                delete m_pRefCount;
            }
        }

    public:
        shared_ptr() : m_pObject(nullptr), m_pRefCount(nullptr) {}

        explicit shared_ptr(T* p) : m_pObject(p), m_pRefCount(new uint32_t(1)) {}

        shared_ptr(const shared_ptr& other) : m_pObject(other.m_pObject), m_pRefCount(other.m_pRefCount)
        {
            if (m_pRefCount)
                ++(*m_pRefCount);
        }

        shared_ptr(shared_ptr&& other) noexcept : m_pObject(std::exchange(other.m_pObject, nullptr)),
            m_pRefCount(std::exchange(other.m_pRefCount, nullptr)) {}

        ~shared_ptr()
        {
            release();
        }

        shared_ptr& operator=(const shared_ptr& other)
        {
            if (this != &other)
            {
                release();

                m_pObject = other.m_pObject;
                m_pRefCount = other.m_pRefCount;

                if (m_pRefCount)
                    ++(*m_pRefCount);
            }

            return *this;
        }

        shared_ptr& operator=(shared_ptr&& other) noexcept
        {
            if (this != &other)
            {
                release();

                m_pObject = std::exchange(other.m_pObject, nullptr);
                m_pRefCount = std::exchange(other.m_pRefCount, nullptr);
            }

            return *this;
        }

        T* get() const { return m_pObject; }

        T& operator*() const { assert(m_pObject); return *m_pObject; }
        T* operator->() const { assert(m_pObject); return m_pObject; }

        explicit operator bool() const { return m_pObject != nullptr; }

        size_t use_count() const { return m_pRefCount ? *m_pRefCount : 0; }
    };
}
