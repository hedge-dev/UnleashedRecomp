#pragma once

#include <SWA.inl>

namespace SWA::Inspire
{
    class CInspireTextureOverlayInfo
    {
    public:
        Hedgehog::Base::CSharedString m_CameraName;
        be<uint32_t> m_Unk1;
        be<uint32_t> m_Unk2;
        be<uint32_t> m_Unk3;
    };
}
