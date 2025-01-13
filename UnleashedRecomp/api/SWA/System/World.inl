namespace SWA
{
    inline boost::shared_ptr<CCamera> CWorld::GetCamera() const
    {
        return m_pMember->m_spOverrideCamera ? m_pMember->m_spOverrideCamera : m_pMember->m_spCamera;
    }
}
