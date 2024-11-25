#pragma once

#include "SWA.inl"
#include "Hedgehog/Base/hhObject.h"

namespace Hedgehog::Mirage
{
    class CRenderable : public Base::CObject
    {
    public:
        xpointer<void> m_pVftable;
        bool m_Enabled;
    };
}
