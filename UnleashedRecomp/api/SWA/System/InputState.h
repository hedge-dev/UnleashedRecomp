#pragma once

#include "SWA.inl"
#include "PadState.h"

namespace SWA
{
    class CInputState : public Hedgehog::Base::CSynchronizedObject
    {
    public:
        // TODO: Hedgehog::Base::TSynchronizedPtr<CInputState>
        static CInputState* GetInstance();

        SPadState m_PadStates[8];
        be<uint32_t> m_CurrentPadStateIndex;

        const SPadState& GetPadState() const;
    };
}

#include "InputState.inl"
