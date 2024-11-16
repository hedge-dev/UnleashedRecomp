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

        CSharedString substr(size_t pos = 0, size_t len = npos) const;

        size_t find(char c, size_t pos = 0) const;
        size_t find(const char* s, size_t pos = 0) const;
        size_t rfind(char c, size_t pos = npos) const;

        size_t find_first_of(const char* s, size_t pos = 0) const;
        size_t find_last_of(const char* s, size_t pos = npos) const;
        size_t find_first_not_of(const char* s, size_t pos = 0) const;
        size_t find_last_not_of(const char* s, size_t pos = npos) const;

        size_t find(const CSharedString& str, size_t pos = 0) const;
        size_t rfind(const CSharedString& str, size_t pos = npos) const;

        size_t find_first_of(const CSharedString& str, size_t pos = 0) const;
        size_t find_last_of(const CSharedString& str, size_t pos = npos) const;
        size_t find_first_not_of(const CSharedString& str, size_t pos = 0) const;
        size_t find_last_not_of(const CSharedString& str, size_t pos = npos) const;

        void assign(const CSharedString& in_rOther);
        void assign(const char* in_pStr);
        void assign(CSharedString&& io_rOther);

        void append(const CSharedString& in_rOther);
        void append(const char* in_pStr);

        void prepend(const CSharedString& in_rOther);
        void prepend(const char* in_pStr);

        int compare(const CSharedString& in_rOther) const;

        CSharedString& operator=(const CSharedString& in_rOther);
        CSharedString& operator=(const char* in_pStr);
        CSharedString& operator=(CSharedString&& io_rOther);
        CSharedString& operator+=(const CSharedString& in_rOther);
        CSharedString& operator+=(const char* in_pStr);

        friend CSharedString operator+(const CSharedString& in_rLeft, const CSharedString& in_rRight);
        friend CSharedString operator+(const CSharedString& in_rLeft, const char* in_pRight);
        friend CSharedString operator+(const char* in_pLeft, const CSharedString& in_pRight);

        bool operator>(const CSharedString& in_rOther) const;
        bool operator>=(const CSharedString& in_rOther) const;
        bool operator<(const CSharedString& in_rOther) const;
        bool operator<=(const CSharedString& in_rOther) const;
        bool operator==(const CSharedString& in_rOther) const;
        bool operator!=(const CSharedString& in_rOther) const;
        bool operator==(const char* in_pOther) const;
        bool operator!=(const char* in_pOther) const;
    };
}

#include <Hedgehog/Base/Type/hhSharedString.inl>
