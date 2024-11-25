namespace SWA
{
    // TODO: Hedgehog::Base::TSynchronizedPtr<CInputState>
    inline CInputState* CInputState::GetInstance()
    {
        return *(xpointer<CInputState>*)MmGetHostAddress(0x83361F5C);
    }

    inline const SPadState& CInputState::GetPadState() const
    {
        return m_PadStates[m_CurrentPadStateIndex];
    }
}
