namespace Hedgehog::Base
{
    inline CStringSymbol::CStringSymbol()
    {
    }

    inline CStringSymbol::CStringSymbol(const char* in_pName)
    {
        GuestToHostFunction<void>(sub_82E014D8, this, in_pName);
    }   
    
    inline CStringSymbol::CStringSymbol(const CSharedString& in_rName)
    {
        GuestToHostFunction<void>(sub_82E013B0, this, &in_rName);
    }

    inline bool CStringSymbol::operator==(const CStringSymbol& in_rOther) const
    {
        return m_Index == in_rOther.m_Index;
    }

    inline bool CStringSymbol::operator!=(const CStringSymbol& in_rOther) const
    {
        return m_Index != in_rOther.m_Index;
    }

    inline bool CStringSymbol::operator<(const CStringSymbol& in_rOther) const
    {
        return m_Index < in_rOther.m_Index;
    }
}
