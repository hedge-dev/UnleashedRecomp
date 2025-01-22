#pragma once

#include <SWA.inl>
#include <SWA/System/GameMode/WorldMap/WorldMapCursor.h>

namespace SWA
{
    class CTitleStateWorldMap : public CTitleStateBase
    {
    public:
        SWA_INSERT_PADDING(0x08);
        xpointer<CWorldMapCursor> m_pWorldMapCursor;
    };
}
