#pragma once

#include <boost/smart_ptr/shared_ptr.h>

#include <Hedgehog/Base/Container/hhVector.h>
#include <Hedgehog/Database/System/hhDatabaseData.h>

namespace Hedgehog::Mirage
{
    class CTextureData;

    class CTexsetData : public Database::CDatabaseData
    {
    public:
        hh::vector<boost::shared_ptr<CTextureData>> m_TextureList;
        hh::vector<Base::CSharedString> m_TextureNameList;
        bool m_ConstTexCoord;
    };

    SWA_ASSERT_OFFSETOF(CTexsetData, m_TextureList, 0xC);
    SWA_ASSERT_OFFSETOF(CTexsetData, m_TextureNameList, 0x1C);
    SWA_ASSERT_OFFSETOF(CTexsetData, m_ConstTexCoord, 0x2C);
    SWA_ASSERT_SIZEOF(CTexsetData, 0x30);
}
