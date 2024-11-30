#pragma once

#include <Hedgehog/Base/Container/hhVector.h>
#include <Hedgehog/Database/System/hhDatabaseData.h>

namespace Hedgehog::Mirage
{
    class CPixelShaderCodeData;
    class CPixelShaderParameterData;

    class CPixelShaderData : public Hedgehog::Database::CDatabaseData
    {
    public:
        boost::shared_ptr<CPixelShaderCodeData> m_spCode;
        SWA_INSERT_PADDING(0x4);
        hh::vector<boost::shared_ptr<CPixelShaderParameterData>> m_ParameterList;            
    };

    SWA_ASSERT_OFFSETOF(CPixelShaderData, m_spCode, 0xC);
    SWA_ASSERT_OFFSETOF(CPixelShaderData, m_ParameterList, 0x18);
    SWA_ASSERT_SIZEOF(CPixelShaderData, 0x28);
}
