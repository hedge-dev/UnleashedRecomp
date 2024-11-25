namespace Hedgehog::Database
{
    inline CDatabaseData::~CDatabaseData()
    {
        GuestToHostFunction<void>(m_pVftable->m_fpDtor, this);
    }

    inline bool CDatabaseData::CheckMadeAll()
    {
        return true;
    }

    inline bool CDatabaseData::IsMadeOne() const
    {
        return (m_Flags & eDatabaseDataFlags_IsMadeOne) != 0;
    }

    inline void CDatabaseData::SetMadeOne()
    {
        m_Flags |= eDatabaseDataFlags_IsMadeOne;
    }

    inline bool CDatabaseData::IsMadeAllInternal()
    {
        if ((m_Flags & eDatabaseDataFlags_IsMadeOne) == 0 || !CheckMadeAll())
            return false;

        m_Flags |= eDatabaseDataFlags_IsMadeAll;
        return true;
    }

    inline bool CDatabaseData::IsMadeAll()
    {
        return (m_Flags & eDatabaseDataFlags_IsMadeAll) != 0 || IsMadeAllInternal();
    }
}
