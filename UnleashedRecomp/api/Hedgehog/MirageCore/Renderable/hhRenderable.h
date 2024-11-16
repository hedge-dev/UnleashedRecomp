#pragma once

#include "SWA.inl"
#include "Hedgehog/Base/hhObject.h"

namespace Hedgehog::Mirage
{
    class CRenderable : public Base::CObject
    {
    public:
        bool m_Enabled;
        be<float> m_SortDepth;
    };
}
