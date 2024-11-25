#pragma once

#include <Hedgehog/Base/Container/hhVector.h>
#include <Hedgehog/Database/System/hhDatabaseData.h>

namespace Hedgehog::Mirage
{
    class CVertexShaderCodeData;
    class CVertexShaderParameterData;

    class CVertexShaderData : public Hedgehog::Database::CDatabaseData
    {
    public:
        boost::shared_ptr<CVertexShaderCodeData> m_spCode;
        SWA_INSERT_PADDING(0x4);
        hh::vector<boost::shared_ptr<CVertexShaderParameterData>> m_ParameterList;
    };

    SWA_ASSERT_OFFSETOF(CVertexShaderData, m_spCode, 0xC);
    SWA_ASSERT_OFFSETOF(CVertexShaderData, m_ParameterList, 0x18);
    SWA_ASSERT_SIZEOF(CVertexShaderData, 0x28);
}
