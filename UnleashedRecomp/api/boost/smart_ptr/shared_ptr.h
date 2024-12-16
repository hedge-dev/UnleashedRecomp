#pragma once

#include <cstddef>
#include <utility>

namespace boost
{
    namespace detail
    {
        class sp_counted_base
        {
        protected:
            struct vftable_t
            {
                be<uint32_t> destructor;
                be<uint32_t> dispose;
                be<uint32_t> destroy;
                be<uint32_t> get_deleter;
            };

            xpointer<vftable_t> vftable_;
            be<uint32_t> use_count_;
            be<uint32_t> weak_count_;

        public:
            // TODO
            sp_counted_base() = delete;

            void add_ref()
            {
                std::atomic_ref useCount(use_count_.value);

                be<uint32_t> original, incremented;
                do
                {
                    original = use_count_;
                    incremented = original + 1;
                } while (!useCount.compare_exchange_weak(original.value, incremented.value));
            }

            void release()
            {
                std::atomic_ref useCount(use_count_.value);

                be<uint32_t> original, decremented;
                do
                {
                    original = use_count_;
                    decremented = original - 1;
                } while (!useCount.compare_exchange_weak(original.value, decremented.value));

                if (decremented == 0)
                {
                    GuestToHostFunction<void>(vftable_->dispose, this);
                    weak_release();
                }
            }

            void weak_release()
            {
                std::atomic_ref weakCount(weak_count_.value);

                be<uint32_t> original, decremented;
                do
                {
                    original = weak_count_;
                    decremented = original - 1;
                } while (!weakCount.compare_exchange_weak(original.value, decremented.value));

                if (decremented == 0)
                {
                    GuestToHostFunction<void>(vftable_->destroy, this);
                }
            }

            uint32_t use_count() const
            {
                return ByteSwap(static_cast<uint32_t const volatile &>(use_count_.value));
            }

            bool unique() const
            {
                return use_count() == 1;
            }
        };

        template< class T > struct sp_dereference
        {
            typedef T& type;
        };

        template<> struct sp_dereference< void >
        {
            typedef void type;
        };
    }

    template<typename T>
    class shared_ptr
    {
    private:
        xpointer<T> px;
        xpointer<boost::detail::sp_counted_base> pn;

        void add_ref()
        {
            if (pn)
                pn->add_ref();
        }

        void release()
        {
            if (pn)
                pn->release();
        }

    public:
        shared_ptr() : px(nullptr), pn(nullptr) {}

        // TODO
        explicit shared_ptr(T* p) = delete;

        shared_ptr(const shared_ptr& other) : px(other.px), pn(other.pn)
        {
            add_ref();
        }

        shared_ptr(shared_ptr&& other) noexcept : px(std::exchange(other.px, nullptr)),
            pn(std::exchange(other.pn, nullptr)) {}

        ~shared_ptr()
        {
            release();
        }

        shared_ptr& operator=(const shared_ptr& other)
        {
            if (this != &other)
            {
                release();

                px = other.px;
                pn = other.pn;

                add_ref();
            }

            return *this;
        }

        shared_ptr& operator=(shared_ptr&& other) noexcept
        {
            if (this != &other)
            {
                release();

                px = std::exchange(other.px, nullptr);
                pn = std::exchange(other.pn, nullptr);
            }

            return *this;
        }

        shared_ptr& operator=(std::nullptr_t)
        {
            release();
            
            px = NULL;
            pn = NULL;

            return *this;
        }

        T* get() const { return px; }

        detail::sp_dereference<T> operator*() const { assert(px); return *px; }
        T* operator->() const { assert(px); return px; }

        explicit operator bool() const { return px != nullptr; }

        size_t use_count() const { return pn ? pn->use_count() : 0; }
        bool unique() const { return !pn || pn->unique(); }
    };

    using anonymous_shared_ptr = shared_ptr<void>;
}
