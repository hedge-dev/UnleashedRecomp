#pragma once
#include "Animation/PathAnimation.h"

namespace SWA
{
    class CPathController
    {
    public:
        be<uint32_t> m_Field000;
        be<uint32_t> m_Field004;
        boost::shared_ptr<PathAnimation::Controller> m_spPathAnimationController;
    };
}
