namespace Chao::CSD
{
    template<typename T>
    inline CResourceBase<T>::~CResourceBase()
    {
        GuestToHostFunction<void>(m_pVftable->m_fpDtor, this);
    }

    template<typename T>
    inline void CResourceBase<T>::CopyResource(const CResourceBase& in_rOther)
    {
        m_rcResourceHolder = in_rOther.m_rcResourceHolder;
        m_pResource = in_rOther.m_pResource;
    }
}
