namespace Chao::CSD
{
    template<typename TObservee>
    inline CObserverBase<TObservee>::~CObserverBase()
    {
        GuestToHostFunction<void>(m_pVftable->m_fpDtor, this);
    }
}
