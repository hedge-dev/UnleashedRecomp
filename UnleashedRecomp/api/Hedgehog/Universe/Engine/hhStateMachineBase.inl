namespace Hedgehog::Universe
{
    inline void* CStateMachineBase::CStateBase::GetContextBase() const
    {
        return m_pContext;
    }

    template<typename T>
    inline T* CStateMachineBase::CStateBase::GetContextBase() const
    {
        return (T*)m_pContext.get();
    }
}
