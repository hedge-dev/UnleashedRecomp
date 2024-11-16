namespace Chao::CSD
{
    template<typename T>
    void CResourceBase<T>::CopyResource(const CResourceBase& in_rOther)
    {
        m_rcResourceHolder = in_rOther.m_rcResourceHolder;
        m_pResource = in_rOther.m_pResource;
    }
}
