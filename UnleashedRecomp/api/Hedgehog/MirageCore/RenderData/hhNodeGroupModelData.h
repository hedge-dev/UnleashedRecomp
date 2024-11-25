#pragma once

#include <Hedgehog/Base/Container/hhVector.h>
#include <Hedgehog/Database/System/hhDatabaseData.h>

namespace Hedgehog::Mirage
{
    class CMeshData;

    class CNodeGroupModelData : public Database::CDatabaseData
    {
    public:
        hh::vector<boost::shared_ptr<CMeshData>> m_OpaqueMeshes;
        hh::vector<boost::shared_ptr<CMeshData>> m_TransparentMeshes;
        hh::vector<boost::shared_ptr<CMeshData>> m_PunchThroughMeshes;
        be<uint32_t> m_SpecialMeshGroupNum;
        boost::shared_ptr<be<uint32_t>> m_SpecialMeshGroupModes;
        hh::vector<hh::vector<boost::shared_ptr<CMeshData>>> m_SpecialMeshGroups;
        Base::CSharedString m_Name;
    };

    SWA_ASSERT_OFFSETOF(CNodeGroupModelData, m_OpaqueMeshes, 0xC);
    SWA_ASSERT_OFFSETOF(CNodeGroupModelData, m_TransparentMeshes, 0x1C);
    SWA_ASSERT_OFFSETOF(CNodeGroupModelData, m_PunchThroughMeshes, 0x2C);
    SWA_ASSERT_OFFSETOF(CNodeGroupModelData, m_SpecialMeshGroupNum, 0x3C);
    SWA_ASSERT_OFFSETOF(CNodeGroupModelData, m_SpecialMeshGroupModes, 0x40);
    SWA_ASSERT_OFFSETOF(CNodeGroupModelData, m_SpecialMeshGroups, 0x48);
    SWA_ASSERT_OFFSETOF(CNodeGroupModelData, m_Name, 0x58);
    SWA_ASSERT_SIZEOF(CNodeGroupModelData, 0x5C);
}
