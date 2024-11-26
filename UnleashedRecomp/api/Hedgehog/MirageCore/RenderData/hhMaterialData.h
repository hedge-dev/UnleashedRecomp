#pragma once

#include <boost/smart_ptr/shared_ptr.h>

#include <Hedgehog/Base/Container/hhVector.h>
#include <Hedgehog/Database/System/hhDatabaseData.h>

namespace Hedgehog::Mirage
{
    class CMaterialData;
    class CTexsetData;
    class CShaderListData;
    class CParameterFloat4Element;
    class CParameterInt4Element;
    class CParameterBoolElement;

    class CMaterialData : public Database::CDatabaseData
    {
    public:
        boost::shared_ptr<CShaderListData> m_spShaderListData;
        boost::shared_ptr<CTexsetData> m_spTexsetData;
        hh::vector<boost::shared_ptr<CParameterFloat4Element>> m_Float4Params;
        hh::vector<boost::shared_ptr<CParameterInt4Element>> m_Int4Params;
        hh::vector<boost::shared_ptr<CParameterBoolElement>> m_Bool4Params;
        uint8_t m_AlphaThreshold;
        bool m_DoubleSided;
        bool m_Additive;
    };

    SWA_ASSERT_OFFSETOF(CMaterialData, m_spShaderListData, 0xC);
    SWA_ASSERT_OFFSETOF(CMaterialData, m_spTexsetData, 0x14);
    SWA_ASSERT_OFFSETOF(CMaterialData, m_Float4Params, 0x1C);
    SWA_ASSERT_OFFSETOF(CMaterialData, m_Int4Params, 0x2C);
    SWA_ASSERT_OFFSETOF(CMaterialData, m_Bool4Params, 0x3C);
    SWA_ASSERT_OFFSETOF(CMaterialData, m_AlphaThreshold, 0x4C);
    SWA_ASSERT_OFFSETOF(CMaterialData, m_DoubleSided, 0x4D);
    SWA_ASSERT_OFFSETOF(CMaterialData, m_Additive, 0x4E);
    SWA_ASSERT_SIZEOF(CMaterialData, 0x50);
}
