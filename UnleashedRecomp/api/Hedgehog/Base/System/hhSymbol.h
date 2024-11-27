#pragma once

#include <SWA.inl>

namespace Hedgehog::Base
{
    class CSharedString;

    class CStringSymbol
    {
    public:
        be<uint32_t> m_Index;

        CStringSymbol();
        CStringSymbol(const char* in_pName);
        CStringSymbol(const CSharedString& in_rName);

        bool operator==(const CStringSymbol& in_rOther) const;
        bool operator!=(const CStringSymbol& in_rOther) const;
        bool operator<(const CStringSymbol& in_rOther) const;
    };

    SWA_ASSERT_OFFSETOF(CStringSymbol, m_Index, 0);
    SWA_ASSERT_SIZEOF(CStringSymbol, 4);
}

#include <Hedgehog/Base/System/hhSymbol.inl>
