#pragma once

#include <boost/smart_ptr/shared_ptr.h>

#include <Hedgehog/Base/Container/hhVector.h>
#include <Hedgehog/Database/System/hhDatabaseData.h>

namespace Hedgehog::Mirage
{
    class CShaderListData;
}

namespace Hedgehog::Sparkle
{
    class CParticleMaterial : public Hedgehog::Database::CDatabaseData
    {
    public:
        enum EBlendMode
        {
            eBlendMode_Zero,
            eBlendMode_Typical,
            eBlendMode_Add,
            eBlendMode_Subtract
        };

        hh::vector<boost::anonymous_shared_ptr> m_spFieldC;
        boost::shared_ptr<Hedgehog::Mirage::CShaderListData> m_spDefaultShaderListData; // BillboardParticle_d[v]
        boost::shared_ptr<Hedgehog::Mirage::CShaderListData> m_spShaderListData;
        bool m_Field2C;
        be<uint32_t> m_BlendMode;
        be<uint32_t> m_AddressMode;
        Hedgehog::Base::CSharedString m_MaterialName;
        Hedgehog::Base::CSharedString m_TextureName;
        Hedgehog::Base::CSharedString m_DeflectionTextureName;
        Hedgehog::Base::CSharedString m_ShaderName;
        be<float> m_Field48;
        be<float> m_Field4C;
    };

    SWA_ASSERT_SIZEOF(CParticleMaterial, 0x50);
}
