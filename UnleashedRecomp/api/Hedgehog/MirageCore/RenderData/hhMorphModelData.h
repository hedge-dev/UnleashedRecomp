#pragma once 

#include <boost/smart_ptr/shared_ptr.h>

#include <Hedgehog/Base/Container/hhVector.h>
#include <Hedgehog/Database/System/hhDatabaseData.h>
#include <Hedgehog/MirageCore/Misc/hhVertexDeclarationPtr.h>

namespace Hedgehog::Mirage
{
    class CMorphTargetData;
    class CMeshIndexData;

    class CMorphModelData : public Hedgehog::Database::CDatabaseData
    {
    public:
        be<uint32_t> m_VertexNum;
        be<uint32_t> m_VertexSize;
        be<uint32_t> m_MorphTargetVertexSize;
        xpointer<void> m_pD3DVertexBuffer;
        CVertexDeclarationPtr m_VertexDeclarationPtr;
        hh::vector<boost::shared_ptr<CMorphTargetData>> m_MorphTargetList;
        hh::vector<boost::shared_ptr<CMeshIndexData>> m_OpaqueMeshList;
        hh::vector<boost::shared_ptr<CMeshIndexData>> m_TransparentMeshList;
        hh::vector<boost::shared_ptr<CMeshIndexData>> m_PunchThroughMeshList;
    };

    SWA_ASSERT_OFFSETOF(CMorphModelData, m_VertexNum, 0xC);
    SWA_ASSERT_OFFSETOF(CMorphModelData, m_VertexSize, 0x10);
    SWA_ASSERT_OFFSETOF(CMorphModelData, m_MorphTargetVertexSize, 0x14);
    SWA_ASSERT_OFFSETOF(CMorphModelData, m_pD3DVertexBuffer, 0x18);
    SWA_ASSERT_OFFSETOF(CMorphModelData, m_VertexDeclarationPtr, 0x1C);
    SWA_ASSERT_OFFSETOF(CMorphModelData, m_MorphTargetList, 0x24);
    SWA_ASSERT_OFFSETOF(CMorphModelData, m_OpaqueMeshList, 0x34);
    SWA_ASSERT_OFFSETOF(CMorphModelData, m_TransparentMeshList, 0x44);
    SWA_ASSERT_OFFSETOF(CMorphModelData, m_PunchThroughMeshList, 0x54);
    SWA_ASSERT_SIZEOF(CMorphModelData, 0x64);
}
