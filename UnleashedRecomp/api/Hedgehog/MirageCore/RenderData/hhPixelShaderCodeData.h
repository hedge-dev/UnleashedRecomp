#pragma once

#include <boost/smart_ptr/shared_ptr.h>

#include <Hedgehog/Database/System/hhDatabaseData.h>

namespace Hedgehog::Base
{
    class CCriticalSectionD3D9;
}

namespace Hedgehog::Mirage
{
    class CRenderingInfrastructure;

    class CPixelShaderCodeData : public Database::CDatabaseData
    {
    public:
        xpointer<void> m_pD3DPixelShader;
        xpointer<uint8_t> m_spFunction;
        boost::shared_ptr<Base::CCriticalSectionD3D9> m_spCriticalSection;
        xpointer<CRenderingInfrastructure> m_pRenderingInfrastructure;
    };

    SWA_ASSERT_OFFSETOF(CPixelShaderCodeData, m_pD3DPixelShader, 0xC);
    SWA_ASSERT_OFFSETOF(CPixelShaderCodeData, m_spFunction, 0x10);
    SWA_ASSERT_OFFSETOF(CPixelShaderCodeData, m_spCriticalSection, 0x14);
    SWA_ASSERT_OFFSETOF(CPixelShaderCodeData, m_pRenderingInfrastructure, 0x1C);
    SWA_ASSERT_SIZEOF(CPixelShaderCodeData, 0x20);
}
