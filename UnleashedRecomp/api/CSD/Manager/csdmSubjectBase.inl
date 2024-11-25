namespace Chao::CSD
{
    template<typename TObserver, typename TObservee>
    inline SubjectBase<TObserver, TObservee>::~SubjectBase()
    {
        GuestToHostFunction<void>(m_pVftable->m_fpDtor, this);
    }

    template<typename TObserver, typename TObservee>
    inline TObservee* SubjectBase<TObserver, TObservee>::GetObservee() const
    {
        return nullptr;
    }
}
