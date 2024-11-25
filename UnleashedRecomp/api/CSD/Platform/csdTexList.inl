namespace Chao::CSD
{
    inline CTexList::~CTexList()
    {
        GuestToHostFunction<void>(m_pVftable->m_fpDtor, this);
    }
}
