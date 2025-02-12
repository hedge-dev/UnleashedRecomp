#pragma once

#include <SWA.inl>
#include "SWA/System/MatrixNodeTransform.h"

namespace SWA::Boss::EggDragoon::DrillMissile
{
    class CMissile
    {
    public:
        SWA_INSERT_PADDING(0xC4);
        xpointer<CMatrixNodeTransform> m_pMatrixNodeTransform;
        SWA_INSERT_PADDING(0x68);
        be<float> m_Speed;
    };
}
