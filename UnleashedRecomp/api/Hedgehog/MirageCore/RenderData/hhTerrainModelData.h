#pragma once

#include <Hedgehog/Base/Container/hhVector.h>
#include <Hedgehog/Database/System/hhDatabaseData.h>

namespace Hedgehog::Mirage
{
    class CSphereData;
    class CNodeGroupModelData;
    class CMeshData;

    class CTerrainModelData : public Database::CDatabaseData
    {
    public:
        SWA_INSERT_PADDING(0x4);
        hh::vector<boost::shared_ptr<CNodeGroupModelData>> m_NodeGroupModels;
        hh::vector<boost::shared_ptr<CMeshData>> m_OpaqueMeshes;
        hh::vector<boost::shared_ptr<CMeshData>> m_TransparentMeshes;
        hh::vector<boost::shared_ptr<CMeshData>> m_PunchThroughMeshes;
        boost::shared_ptr<CSphereData> m_spSphere;
    };

    SWA_ASSERT_OFFSETOF(CTerrainModelData, m_NodeGroupModels, 0x10);
    SWA_ASSERT_OFFSETOF(CTerrainModelData, m_OpaqueMeshes, 0x20);
    SWA_ASSERT_OFFSETOF(CTerrainModelData, m_TransparentMeshes, 0x30);
    SWA_ASSERT_OFFSETOF(CTerrainModelData, m_PunchThroughMeshes, 0x40);
    SWA_ASSERT_OFFSETOF(CTerrainModelData, m_spSphere, 0x50);
    SWA_ASSERT_SIZEOF(CTerrainModelData, 0x58);
}
