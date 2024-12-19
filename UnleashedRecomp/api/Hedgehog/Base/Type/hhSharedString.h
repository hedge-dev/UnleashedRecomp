#pragma once

#include <Hedgehog/Base/Type/detail/hhStringHolder.h>

namespace Hedgehog::Base
{
    class CSharedString
    {
    private:
        xpointer<const char> m_pStr;

        SStringHolder* GetHolder() const;

        CSharedString(SStringHolder* in_pHolder);

    public:
        static constexpr size_t npos = ~0u;

        CSharedString();
        CSharedString(const char* in_pStr);
        CSharedString(const CSharedString& in_rOther);
        CSharedString(CSharedString&& io_rOther);
        ~CSharedString();

        const char* get() const;
        const char* c_str() const;
        const char* data() const;

        size_t size() const;
        size_t length() const;
        bool empty() const;

        const char* begin() const;
        const char* end() const;
    };
}

#include <Hedgehog/Base/Type/hhSharedString.inl>
