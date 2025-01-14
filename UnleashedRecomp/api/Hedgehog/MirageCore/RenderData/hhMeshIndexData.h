#pragma once

#include <boost/smart_ptr/shared_ptr.h>
#include <Hedgehog/Database/System/hhDatabaseData.h>

namespace Hedgehog::Mirage
{
    class CMaterialData;

    class CMeshIndexData : public Hedgehog::Database::CDatabaseData
    {
    public:
        be<uint32_t> m_IndexNum;
        be<uint32_t> m_NodeNum;
        xpointer<uint8_t> m_pNodeIndices;
        xpointer<void> m_pD3DIndexBuffer;
        boost::shared_ptr<CMaterialData> m_spMaterial;
    };

    SWA_ASSERT_OFFSETOF(CMeshIndexData, m_IndexNum, 0xC);
    SWA_ASSERT_OFFSETOF(CMeshIndexData, m_NodeNum, 0x10);
    SWA_ASSERT_OFFSETOF(CMeshIndexData, m_pNodeIndices, 0x14);
    SWA_ASSERT_OFFSETOF(CMeshIndexData, m_pD3DIndexBuffer, 0x18);
    SWA_ASSERT_OFFSETOF(CMeshIndexData, m_spMaterial, 0x1C);
    SWA_ASSERT_SIZEOF(CMeshIndexData, 0x24);
}
