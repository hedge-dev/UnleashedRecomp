#pragma once

#include <SWA.inl>
#include <SWA/Inspire/InspireTextureOverlayInfo.h>

namespace SWA::Inspire
{
    class CScene;

    class CTextureOverlay
    {
    public:
        xpointer<void> m_pVftable;
        boost::shared_ptr<CInspireTextureOverlayInfo> m_spInfo;
        xpointer<CScene> m_pScene;
        boost::shared_ptr<Hedgehog::Mirage::CTextureData> m_spTextureData;
    };
}
