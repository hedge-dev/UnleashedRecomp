namespace Chao::CSD
{
    inline RCPtr<CNode> CScene::GetNode(const char* in_pName) const
    {
        RCPtr<CNode> rcNode;
        GuestToHostFunction<void>(0x830BCCA8, this, rcNode, in_pName);
        return rcNode;
    }

    inline bool CScene::SetMotion(const char* in_pName)
    {
        return GuestToHostFunction<bool>(0x830BA760, this, in_pName);
    }

    inline void CScene::SetMotionFrame(float in_MotionFrame)
    {
        m_PrevMotionFrame = in_MotionFrame;
        m_MotionFrame = in_MotionFrame;
    }

    inline void CScene::SetPosition(float in_X, float in_Y)
    {
        GuestToHostFunction<void>(0x830BB550, this, in_X, in_Y);
    }

    inline void CScene::SetHideFlag(uint32_t in_HideFlag)
    {
        GuestToHostFunction<void>(0x830BB378, this, in_HideFlag);
    }

    inline void CScene::SetRotation(float in_Angle)
    {
        GuestToHostFunction<void>(0x830BB5F8, this, in_Angle);
    }

    inline void CScene::SetScale(float in_X, float in_Y)
    {
        GuestToHostFunction<void>(0x830BB650, this, in_X, in_Y);
    }
}
