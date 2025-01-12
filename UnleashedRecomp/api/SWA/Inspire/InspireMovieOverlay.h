#pragma once

#include <SWA.inl>
#include <SWA/Inspire/InspireMovieOverlayInfo.h>

namespace SWA::Inspire
{
    class CScene;

    class CMovieOverlay
    {
    public:
        boost::shared_ptr<CMovieOverlayInfo> m_spInfo;
        xpointer<CScene> m_pScene;
        xpointer<Hedgehog::Mirage::CTextureData> m_pTextureData;
    };
}
