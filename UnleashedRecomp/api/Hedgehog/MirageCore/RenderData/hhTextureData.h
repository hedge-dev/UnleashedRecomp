#pragma once

#include <Hedgehog/Database/System/hhDatabaseData.h>

namespace Hedgehog::Mirage
{
    class CTextureData : public Database::CDatabaseData
    {
    public:
        SWA_INSERT_PADDING(0x8);
        uint8_t m_TexcoordIndex;
        SWA_INSERT_PADDING(0x1B);
    };

    SWA_ASSERT_OFFSETOF(CTextureData, m_TexcoordIndex, 0x14);
    SWA_ASSERT_SIZEOF(CTextureData, 0x30);
}
