#pragma once

#include <SWA.inl>

namespace SWA
{
    enum ELoadingDisplayType
    {
        eLoadingDisplayType_MilesElectric,
        eLoadingDisplayType_None,
        eLoadingDisplayType_WerehogMovie,
        eLoadingDisplayType_MilesElectricContext,
        eLoadingDisplayType_Arrows,
        eLoadingDisplayType_NowLoading,
        eLoadingDisplayType_EventGallery,
        eLoadingDisplayType_ChangeTimeOfDay,
        eLoadingDisplayType_Blank
    };

    class CLoading
    {
    public:
        SWA_INSERT_PADDING(0xD8);
        be<uint32_t> m_FieldD8;
        SWA_INSERT_PADDING(0x3C);
        Chao::CSD::RCPtr<Chao::CSD::CScene> m_rcNightToDay;
        SWA_INSERT_PADDING(0x0C);
        be<uint32_t> m_IsVisible;
        SWA_INSERT_PADDING(0x0C);
        be<ELoadingDisplayType> m_LoadingDisplayType;
        SWA_INSERT_PADDING(0x61);
        bool m_IsNightToDay;
    };

    SWA_ASSERT_OFFSETOF(CLoading, m_FieldD8, 0xD8);
    SWA_ASSERT_OFFSETOF(CLoading, m_rcNightToDay, 0x118);
    SWA_ASSERT_OFFSETOF(CLoading, m_IsVisible, 0x12C);
    SWA_ASSERT_OFFSETOF(CLoading, m_LoadingDisplayType, 0x13C);
    SWA_ASSERT_OFFSETOF(CLoading, m_IsNightToDay, 0x1A1);
}
