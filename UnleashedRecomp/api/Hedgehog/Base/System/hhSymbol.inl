namespace Hedgehog::Base
{
    inline const char* SSymbolNode::GetValue() const
    {
        return reinterpret_cast<const char*>(this) + sizeof(SSymbolNode);
    }

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

    inline const char* CStringSymbol::GetValue() const
    {
        return m_pSymbolNode->GetValue();
    }

    inline bool CStringSymbol::operator==(const CStringSymbol& in_rOther) const
    {
        return m_pSymbolNode == in_rOther.m_pSymbolNode;
    }

    inline bool CStringSymbol::operator!=(const CStringSymbol& in_rOther) const
    {
        return m_pSymbolNode != in_rOther.m_pSymbolNode;
    }

    inline bool CStringSymbol::operator<(const CStringSymbol& in_rOther) const
    {
        return m_pSymbolNode < in_rOther.m_pSymbolNode;
    }
}
