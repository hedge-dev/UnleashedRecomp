#pragma once

#include <boost/smart_ptr/shared_ptr.h>

#include <Hedgehog/Base/Container/hhMap.h>
#include <Hedgehog/Base/System/hhSymbol.h>
#include <Hedgehog/Database/System/hhDatabaseData.h>

namespace Hedgehog::Mirage
{
    class CVertexShaderData;
    class CPixelShaderData;

    class CVertexShaderPermutationData
    {
    public:
        hh::map<be<uint32_t>, boost::shared_ptr<CVertexShaderData>> m_VertexShaders;
        be<uint32_t> m_SubPermutations;
    };

    SWA_ASSERT_OFFSETOF(CVertexShaderPermutationData, m_VertexShaders, 0x0);
    SWA_ASSERT_OFFSETOF(CVertexShaderPermutationData, m_SubPermutations, 0xC);
    SWA_ASSERT_SIZEOF(CVertexShaderPermutationData, 0x10);

    class CPixelShaderPermutationData
    {
    public:
        hh::map<Base::CStringSymbol, boost::shared_ptr<CVertexShaderPermutationData>> m_VertexShaderPermutations;
        hh::map<be<uint32_t>, boost::shared_ptr<CPixelShaderData>> m_PixelShaders;
        be<uint32_t> m_SubPermutations;
    };

    SWA_ASSERT_OFFSETOF(CPixelShaderPermutationData, m_VertexShaderPermutations, 0x0);
    SWA_ASSERT_OFFSETOF(CPixelShaderPermutationData, m_PixelShaders, 0xC);
    SWA_ASSERT_OFFSETOF(CPixelShaderPermutationData, m_SubPermutations, 0x18);
    SWA_ASSERT_SIZEOF(CPixelShaderPermutationData, 0x1C);

    class CShaderListData : public Database::CDatabaseData
    {
    public:
        hh::map<Base::CStringSymbol, CPixelShaderPermutationData> m_PixelShaderPermutations;
    };

    SWA_ASSERT_OFFSETOF(CShaderListData, m_PixelShaderPermutations, 0xC);
    SWA_ASSERT_SIZEOF(CShaderListData, 0x18);
}
