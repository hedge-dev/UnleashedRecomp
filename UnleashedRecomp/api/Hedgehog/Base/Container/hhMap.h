#pragma once

#include <Hedgehog/Base/System/hhAllocator.h>

namespace hh
{
    template<
        class Key, 
        class T, 
        class Compare = std::less<Key>, 
        class Allocator = Hedgehog::Base::TAllocator<std::pair<const Key, T>>>
    class map
    {
    protected:
        enum EColor
        {
            eColor_Red,
            eColor_Black
        };

        struct SNode
        {
            using allocator_type = typename std::allocator_traits<Allocator>::template rebind_alloc<SNode>;

            xpointer<SNode> pLeft;
            xpointer<SNode> pParent;
            xpointer<SNode> pRight;
            std::pair<const Key, T> Value;
            uint8_t Color;
            bool IsNil;
        };

        Compare m_Comp;
        typename SNode::allocator_type m_Alloc;
        xpointer<SNode> m_pHead;
        be<uint32_t> m_Count;

        struct SFindResult
        {
            SNode* pParent;
            bool IsRight;
            SNode* pBound;
        };

        bool LowerBoundDuplicate(const SNode* pBound, const Key& in_rKey) const
        {
            return !pBound->IsNil && !m_Comp(in_rKey, pBound->Value.first);
        }

        SFindResult FindLowerBound(const Key& in_rKey) const
        {
            SFindResult result{ m_pHead->pParent, true, m_pHead };
            SNode* pNode = result.pParent;

            while (!pNode->IsNil)
            {
                result.pParent = pNode;
                if (m_Comp(pNode->Value.first, in_rKey))
                {
                    result.IsRight = true;
                    pNode = pNode->pRight;
                }
                else
                {
                    result.IsRight = false;
                    result.pBound = pNode;
                    pNode = pNode->pLeft;
                }
            }

            return result;
        }

        SNode* Find(const Key& in_rKey) const
        {
            const SFindResult result = FindLowerBound(in_rKey);
            return LowerBoundDuplicate(result.pBound, in_rKey) ? result.pBound : m_pHead.get();
        }

        static SNode* Max(SNode* pNode)
        {
            while (!pNode->pRight->IsNil)
                pNode = pNode->pRight;

            return pNode;
        }

        static SNode* Min(SNode* pNode)
        {
            while (!pNode->pLeft->IsNil)
                pNode = pNode->pLeft;

            return pNode;
        }

    public:
        using key_type = Key;
        using mapped_type = T;
        using value_type = std::pair<const Key, T>;
        using size_type = std::size_t;
        using difference_type = std::ptrdiff_t;
        using key_compare = Compare;
        using allocator_type = Allocator;
        using reference = value_type&;
        using const_reference = const value_type&;
        using pointer = typename std::allocator_traits<Allocator>::pointer;
        using const_pointer = typename std::allocator_traits<Allocator>::const_pointer;

        class iterator
        {
        public:
            using iterator_category = std::bidirectional_iterator_tag;
            using value_type = std::pair<const Key, T>;
            using difference_type = std::ptrdiff_t;
            using pointer = value_type*;
            using reference = value_type&;

            iterator(const std::nullptr_t&) = delete;
            iterator(SNode* pNode) : m_pNode(pNode) {}

            reference operator*() const { return m_pNode->Value; }
            pointer operator->() const { return &m_pNode->Value; }

            iterator& operator++()
            {
                if (m_pNode->pRight->IsNil)
                {
                    SNode* pNode;
                    while (!(pNode = m_pNode->pParent)->IsNil && m_pNode == pNode->pRight)
                        m_pNode = pNode;

                    m_pNode = pNode;
                }
                else
                {
                    m_pNode = map::Min(m_pNode->pRight);
                }

                return *this;
            }

            iterator operator++(int)
            {
                iterator temp(*this);
                ++(*this);
                return temp;
            }

            iterator& operator--()
            {
                if (m_pNode->IsNil)
                {
                    m_pNode = m_pNode->pRight;
                }
                else if (m_pNode->pLeft->IsNil)
                {
                    SNode* pNode;
                    while (!(pNode = m_pNode->pParent)->IsNil && m_pNode == pNode->pLeft)
                        m_pNode = pNode;

                    if (!m_pNode->IsNil)
                        m_pNode = pNode;
                }
                else
                {
                    m_pNode = map::Max(m_pNode->pLeft);
                }

                return *this;
            }

            iterator operator--(int)
            {
                iterator temp(*this);
                --(*this);
                return temp;
            }

            bool operator==(const iterator& rhs) const { return m_pNode == rhs.m_pNode; }
            bool operator!=(const iterator& rhs) const { return !(*this == rhs); }

        private:
            SNode* m_pNode;
            friend class iterator;
            friend class map;
        };

        class const_iterator
        {
        public:
            using iterator_category = std::bidirectional_iterator_tag;
            using value_type = std::pair<const Key, T>;
            using difference_type = std::ptrdiff_t;
            using pointer = const value_type*;
            using reference = const value_type&;

            const_iterator(const std::nullptr_t&) = delete;
            const_iterator(SNode* pNode) : m_pNode(pNode) {}
            const_iterator(const iterator& iterator) : m_pNode(iterator.m_pNode) {}

            reference operator*() const { return m_pNode->Value; }
            pointer operator->() const { return &m_pNode->Value; }

            const_iterator& operator++()
            {
                if (m_pNode->pRight->IsNil)
                {
                    SNode* pNode;
                    while (!(pNode = m_pNode->pParent)->IsNil && m_pNode == pNode->pRight)
                        m_pNode = pNode;

                    m_pNode = pNode;
                }
                else
                {
                    m_pNode = map::Min(m_pNode->pRight);
                }

                return *this;
            }

            const_iterator operator++(int)
            {
                const_iterator temp(*this);
                ++(*this);
                return temp;
            }

            const_iterator& operator--()
            {
                if (m_pNode->IsNil)
                {
                    m_pNode = m_pNode->pRight;
                }
                else if (m_pNode->pLeft->IsNil)
                {
                    SNode* pNode;
                    while (!(pNode = m_pNode->pParent)->IsNil && m_pNode == pNode->pLeft)
                        m_pNode = pNode;

                    if (!m_pNode->IsNil)
                        m_pNode = pNode;
                }
                else
                {
                    m_pNode = map::Max(m_pNode->pLeft);
                }

                return *this;
            }

            const_iterator operator--(int)
            {
                const_iterator temp(*this);
                --(*this);
                return temp;
            }

            bool operator==(const const_iterator& rhs) const { return m_pNode == rhs.m_pNode; }
            bool operator!=(const const_iterator& rhs) const { return !(*this == rhs); }

        private:
            SNode* m_pNode;
            friend class map;
        };

        using reverse_iterator = std::reverse_iterator<iterator>;
        using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    public:
        allocator_type get_allocator() const
        {
            return m_Alloc;
        }

        T& at(const Key& key)
        {
            return Find(key)->Value.second;
        }

        const T& at(const Key& key) const
        {
            return Find(key)->Value.second;
        }

        iterator begin()
        {
            return iterator(m_pHead->pLeft);
        }

        const_iterator begin() const
        {
            return const_iterator(m_pHead->pLeft);
        }

        const_iterator cbegin() const
        {
            return const_iterator(m_pHead->pLeft);
        }

        iterator end()
        {
            return iterator(m_pHead);
        }

        const_iterator end() const
        {
            return const_iterator(m_pHead);
        }

        const_iterator cend() const
        {
            return const_iterator(m_pHead);
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
            return m_Count == 0;
        }

        size_type size() const
        {
            return m_Count;
        }

        size_type max_size() const
        {
            return ~0u;
        }

        size_type count(const Key& key) const
        {
            return LowerBoundDuplicate(FindLowerBound(key).pBound, key) ? 1u : 0u;
        }

        iterator find(const Key& key)
        {
            return iterator(Find(key));
        }

        const_iterator find(const Key& key) const
        {
            return const_iterator(Find(key));
        }
    };

    static_assert(sizeof(map<int, int>) == 0xC);
}
