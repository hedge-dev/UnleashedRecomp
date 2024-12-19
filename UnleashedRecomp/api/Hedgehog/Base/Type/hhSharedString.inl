namespace Hedgehog::Base
{
    inline SStringHolder* CSharedString::GetHolder() const
    {
        return SStringHolder::GetHolder(m_pStr);
    }

    inline CSharedString::CSharedString(SStringHolder* in_pHolder) : m_pStr(in_pHolder->aStr)
    {
    }

    inline CSharedString::CSharedString() : m_pStr(nullptr)
    {
    }

    inline CSharedString::CSharedString(const char* in_pStr) : m_pStr(nullptr)
    {
        size_t length;

        if (in_pStr && (length = strlen(in_pStr)) != 0)
            m_pStr.ptr = g_memory.MapVirtual(SStringHolder::Make(in_pStr)->aStr);
    }

    inline CSharedString::CSharedString(const CSharedString& in_rOther) : m_pStr(in_rOther.m_pStr)
    {
        GetHolder()->AddRef();
    }

    inline CSharedString::CSharedString(CSharedString&& io_rOther) : m_pStr(io_rOther.m_pStr)
    {
        io_rOther.m_pStr = nullptr;
    }

    inline CSharedString::~CSharedString()
    {
        GetHolder()->Release();
    }

    inline const char* CSharedString::get() const
    {
        return m_pStr;
    }

    inline const char* CSharedString::c_str() const
    {
        return get();
    }

    inline const char* CSharedString::data() const
    {
        return get();
    }

    inline size_t CSharedString::size() const
    {
        return strlen(m_pStr);
    }

    inline size_t CSharedString::length() const
    {
        return size();
    }

    inline bool CSharedString::empty() const
    {
        return size() == 0;
    }

    inline const char* CSharedString::begin() const
    {
        return get();
    }

    inline const char* CSharedString::end() const
    {
        return &m_pStr[size()];
    }
}
