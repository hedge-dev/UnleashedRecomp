#pragma once

#include <SWA.inl>

namespace SWA
{
    class CSaveIcon : Hedgehog::Universe::CUpdateUnit
    {
    public:
        SWA_INSERT_PADDING(0xD8);
        bool m_IsVisible;
    };
}
