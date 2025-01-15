#pragma once

#include <SWA.inl>

namespace SWA::Inspire
{
    struct SSceneData
    {
        be<float> Frame;
        be<uint32_t> Cut;
        bool IsPlaying;
        SWA_INSERT_PADDING(0x177);
    };

    class CScene
    {
    public:
        SWA_INSERT_PADDING(0xC0);
        xpointer<SSceneData> m_pData;
    };
}
