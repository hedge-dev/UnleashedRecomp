#pragma once

#include <Hedgehog/Base/Container/hhVector.h>
#include <Hedgehog/Database/System/hhDatabaseData.h>

namespace Hedgehog::Mirage
{
    class CNodeGroupModelData;
    class CMeshData;
    class CModelNodeData;
    class CMatrixData;
    class CAabbData;
    class CSphereData;
    class CMorphModelData;

    class CModelData : public Database::CDatabaseData
    {
    public:
        be<uint32_t> m_NodeGroupModelNum;
        hh::vector<boost::shared_ptr<CNodeGroupModelData>> m_NodeGroupModels;
        hh::vector<boost::shared_ptr<CMeshData>> m_OpaqueMeshes;
        hh::vector<boost::shared_ptr<CMeshData>> m_TransparentMeshes;
        hh::vector<boost::shared_ptr<CMeshData>> m_PunchThroughMeshes;
        be<uint32_t> m_NodeNum;
        boost::shared_ptr<uint8_t[]> m_spNodeParentIndices;
        boost::shared_ptr<CModelNodeData[]> m_spNodes;
        boost::shared_ptr<CMatrixData[]> m_spNodeMatrices;
        boost::shared_ptr<CAabbData> m_spAabb;
        boost::shared_ptr<CSphereData> m_spSphere;
        hh::vector<boost::shared_ptr<CMorphModelData>> m_MorphModels;
    };

    SWA_ASSERT_OFFSETOF(CModelData, m_NodeGroupModelNum, 0xC);
    SWA_ASSERT_OFFSETOF(CModelData, m_NodeGroupModels, 0x10);
    SWA_ASSERT_OFFSETOF(CModelData, m_OpaqueMeshes, 0x20);
    SWA_ASSERT_OFFSETOF(CModelData, m_TransparentMeshes, 0x30);
    SWA_ASSERT_OFFSETOF(CModelData, m_PunchThroughMeshes, 0x40);
    SWA_ASSERT_OFFSETOF(CModelData, m_NodeNum, 0x50);
    SWA_ASSERT_OFFSETOF(CModelData, m_spNodeParentIndices, 0x54);
    SWA_ASSERT_OFFSETOF(CModelData, m_spNodes, 0x5C);
    SWA_ASSERT_OFFSETOF(CModelData, m_spNodeMatrices, 0x64);
    SWA_ASSERT_OFFSETOF(CModelData, m_spAabb, 0x6C);
    SWA_ASSERT_OFFSETOF(CModelData, m_spSphere, 0x74);
    SWA_ASSERT_OFFSETOF(CModelData, m_MorphModels, 0x7C);
    SWA_ASSERT_SIZEOF(CModelData, 0x8C);
}
