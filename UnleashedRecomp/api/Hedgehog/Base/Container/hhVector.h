#pragma once

#include <Hedgehog/Base/System/hhAllocator.h>

namespace hh
{
    template<typename T, typename Allocator = Hedgehog::Base::TAllocator<T>>
    class vector
    {
    protected:
        Allocator m_Alloc;
        xpointer<T> m_pFirst;
        xpointer<T> m_pLast;
        xpointer<T> m_pEnd;

    public:
        using value_type = T;
        using allocator_type = Allocator;
        using size_type = std::size_t;
        using difference_type = std::ptrdiff_t;
        using reference = value_type&;
        using const_reference = const value_type&;
        using pointer = typename std::allocator_traits<Allocator>::pointer;
        using const_pointer = typename std::allocator_traits<Allocator>::const_pointer;

        class iterator
        {
        public:
            using iterator_category = std::random_access_iterator_tag;
            using value_type = T;
            using difference_type = std::ptrdiff_t;
            using pointer = T*;
            using reference = T&;

            iterator() : m_pPtr(nullptr) {}
            iterator(T* p) : m_pPtr(p) {}

            reference operator*() const { return *m_pPtr; }
            pointer operator->() const { return m_pPtr; }

            iterator& operator++() { ++m_pPtr; return *this; }
            iterator operator++(int) { iterator tmp = *this; ++(*this); return tmp; }

            iterator& operator--() { --m_pPtr; return *this; }
            iterator operator--(int) { iterator tmp = *this; --(*this); return tmp; }

            iterator& operator+=(difference_type n) { m_pPtr += n; return *this; }
            iterator operator+(difference_type n) const { iterator tmp = *this; return tmp += n; }
            friend iterator operator+(difference_type n, iterator it) { return it + n; }

            iterator& operator-=(difference_type n) { return *this += -n; }
            iterator operator-(difference_type n) const { iterator tmp = *this; return tmp -= n; }
            difference_type operator-(const iterator& other) const { return m_pPtr - other.m_pPtr; }

            reference operator[](difference_type n) const { return *(*this + n); }

            bool operator==(const iterator& other) const { return m_pPtr == other.m_pPtr; }
            bool operator!=(const iterator& other) const { return !(*this == other); }
            bool operator<(const iterator& other) const { return m_pPtr < other.m_pPtr; }
            bool operator<=(const iterator& other) const { return !(other < *this); }
            bool operator>(const iterator& other) const { return other < *this; }
            bool operator>=(const iterator& other) const { return !(*this < other); }

        private:
            T* m_pPtr;

            friend class vector;
            friend class const_iterator;
        };

        class const_iterator
        {
        public:
            using iterator_category = std::random_access_iterator_tag;
            using value_type = T;
            using difference_type = std::ptrdiff_t;
            using pointer = const T*;
            using reference = const T&;

            const_iterator() : m_pPtr(nullptr) {}
            const_iterator(T* p) : m_pPtr(p) {}
            const_iterator(const iterator& other) : m_pPtr(other.m_pPtr) {}

            reference operator*() const { return *m_pPtr; }
            pointer operator->() const { return m_pPtr; }

            const_iterator& operator++() { ++m_pPtr; return *this; }
            const_iterator operator++(int) { const_iterator tmp = *this; ++(*this); return tmp; }

            const_iterator& operator--() { --m_pPtr; return *this; }
            const_iterator operator--(int) { const_iterator tmp = *this; --(*this); return tmp; }

            const_iterator& operator+=(difference_type n) { m_pPtr += n; return *this; }
            const_iterator operator+(difference_type n) const { const_iterator tmp = *this; return tmp += n; }
            friend const_iterator operator+(difference_type n, const_iterator it) { return it + n; }

            const_iterator& operator-=(difference_type n) { return *this += -n; }
            const_iterator operator-(difference_type n) const { const_iterator tmp = *this; return tmp -= n; }
            difference_type operator-(const const_iterator& other) const { return m_pPtr - other.m_pPtr; }

            reference operator[](difference_type n) const { return *(*this + n); }

            bool operator==(const const_iterator& other) const { return m_pPtr == other.m_pPtr; }
            bool operator!=(const const_iterator& other) const { return !(*this == other); }
            bool operator<(const const_iterator& other) const { return m_pPtr < other.m_pPtr; }
            bool operator<=(const const_iterator& other) const { return !(other < *this); }
            bool operator>(const const_iterator& other) const { return other < *this; }
            bool operator>=(const const_iterator& other) const { return !(*this < other); }

        private:
            T* m_pPtr;
            friend class vector;
        };

        using reverse_iterator = std::reverse_iterator<iterator>;
        using const_reverse_iterator = std::reverse_iterator<const_iterator>;

        allocator_type get_allocator() const
        {
            return m_Alloc;
        }

        reference at(size_type pos)
        {
            return m_pFirst[pos];
        }

        const_reference at(size_type pos) const
        {
            return m_pFirst[pos];
        }

        reference operator[](size_type pos)
        {
            return m_pFirst[pos];
        }

        const_reference operator[](size_type pos) const
        {
            return m_pFirst[pos];
        }

        reference front()
        {
            return m_pFirst[0];
        }

        const_reference front() const
        {
            return m_pFirst[0];
        }

        reference back()
        {
            return m_pLast[-1];
        }

        const_reference back() const
        {
            return m_pLast[-1];
        }

        T* data()
        {
            return m_pFirst;
        }

        const T* data() const
        {
            return m_pFirst;
        }

        iterator begin()
        {
            return iterator(m_pFirst);
        }

        const_iterator begin() const
        {
            return const_iterator(m_pFirst);
        }

        const_iterator cbegin() const
        {
            return const_iterator(m_pFirst);
        }

        iterator end()
        {
            return iterator(m_pLast);
        }

        const_iterator end() const
        {
            return const_iterator(m_pLast);
        }

        const_iterator cend() const
        {
            return const_iterator(m_pLast);
        }

        reverse_iterator rbegin()
        {
            return reverse_iterator(end());
        }

        const_reverse_iterator rbegin() const
        {
            return const_reverse_iterator(end());
        }

        const_reverse_iterator crbegin() const
        {
            return const_reverse_iterator(cend());
        }

        reverse_iterator rend()
        {
            return reverse_iterator(begin());
        }

        const_reverse_iterator rend() const
        {
            return const_reverse_iterator(begin());
        }

        const_reverse_iterator crend() const
        {
            return const_reverse_iterator(cbegin());
        }

        bool empty() const
        {
            return m_pFirst == m_pLast;
        }

        size_type size() const
        {
            return m_pLast - m_pFirst;
        }

        size_type max_size() const
        {
            return ~0u;
        }

        size_type capacity() const
        {
            return m_pEnd - m_pFirst;
        }
    };

    SWA_ASSERT_SIZEOF(vector<int>, 0x10);
}
