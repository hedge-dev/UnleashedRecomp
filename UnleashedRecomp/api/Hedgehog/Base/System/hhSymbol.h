#pragma once

#include <SWA.inl>

namespace Hedgehog::Base
{
    struct SSymbolNode
    {
        xpointer<SSymbolNode> pPrev;
        xpointer<SSymbolNode> pNext;
        be<uint32_t> Hash;

        const char* GetValue() const;
    };

    class CSharedString;

    class CStringSymbol
    {
    public:
        xpointer<SSymbolNode> m_pSymbolNode;

        CStringSymbol();
        CStringSymbol(const char* in_pName);
        CStringSymbol(const CSharedString& in_rName);

        const char* GetValue() const;

        bool operator==(const CStringSymbol& in_rOther) const;
        bool operator!=(const CStringSymbol& in_rOther) const;
        bool operator<(const CStringSymbol& in_rOther) const;
    };

    SWA_ASSERT_OFFSETOF(CStringSymbol, m_pSymbolNode, 0);
    SWA_ASSERT_SIZEOF(CStringSymbol, 4);
}

#include <Hedgehog/Base/System/hhSymbol.inl>
