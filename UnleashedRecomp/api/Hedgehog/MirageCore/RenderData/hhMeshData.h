#pragma once

#include <boost/smart_ptr/shared_ptr.h>

#include <Hedgehog/Base/Type/hhSharedString.h>
#include <Hedgehog/Database/System/hhDatabaseData.h>
#include <Hedgehog/MirageCore/Misc/hhVertexDeclarationPtr.h>

namespace Hedgehog::Mirage
{
    class CMaterialData;

    class CMeshData : public Database::CDatabaseData
    {
    public:
        be<uint32_t> m_IndexNum;
        be<uint32_t> m_VertexNum;
        be<uint32_t> m_VertexSize;
        be<uint32_t> m_NodeNum;
        xpointer<uint8_t> m_pNodeIndices;
        be<uint32_t> m_VertexOffset;
        be<uint32_t> m_IndexOffset;
        xpointer<void> m_pD3DIndexBuffer;
        xpointer<void> m_pD3DVertexBuffer;
        CVertexDeclarationPtr m_VertexDeclarationPtr;
        SWA_INSERT_PADDING(0x4);
        boost::shared_ptr<CMaterialData> m_spMaterial;
        SWA_INSERT_PADDING(0xC);
    };
    
    SWA_ASSERT_OFFSETOF(CMeshData, m_IndexNum, 0xC);
    SWA_ASSERT_OFFSETOF(CMeshData, m_VertexNum, 0x10);
    SWA_ASSERT_OFFSETOF(CMeshData, m_VertexSize, 0x14);
    SWA_ASSERT_OFFSETOF(CMeshData, m_NodeNum, 0x18);
    SWA_ASSERT_OFFSETOF(CMeshData, m_pNodeIndices, 0x1C);
    SWA_ASSERT_OFFSETOF(CMeshData, m_VertexOffset, 0x20);
    SWA_ASSERT_OFFSETOF(CMeshData, m_IndexOffset, 0x24);
    SWA_ASSERT_OFFSETOF(CMeshData, m_pD3DIndexBuffer, 0x28);
    SWA_ASSERT_OFFSETOF(CMeshData, m_pD3DVertexBuffer, 0x2C);
    SWA_ASSERT_OFFSETOF(CMeshData, m_VertexDeclarationPtr, 0x30);
    SWA_ASSERT_OFFSETOF(CMeshData, m_spMaterial, 0x3C);
    SWA_ASSERT_SIZEOF(CMeshData, 0x50);
}
