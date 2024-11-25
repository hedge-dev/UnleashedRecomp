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
        SWA_INSERT_PADDING(0x128);
        bool m_IsVisible;
        SWA_INSERT_PADDING(0x0C);
        be<ELoadingDisplayType> m_LoadingDisplayType;
        SWA_INSERT_PADDING(0x65);
        bool m_IsNightToDay;
    };
}
