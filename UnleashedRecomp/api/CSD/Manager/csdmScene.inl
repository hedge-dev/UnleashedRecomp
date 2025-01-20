namespace Chao::CSD
{
    inline CScene::~CScene()
    {
        SWA_VIRTUAL_FUNCTION(void, 0, this);
    }

    inline void CScene::Update(float in_DeltaTime)
    {
        SWA_VIRTUAL_FUNCTION(void, 2, this, in_DeltaTime);
    }

    inline void CScene::Render(void* in_pUnk)
    {
        SWA_VIRTUAL_FUNCTION(void, 3, this, in_pUnk);
    }

    inline void CScene::GetNode(RCPtr<CNode>& out_rResult, const char* in_pName)
    {
        GuestToHostFunction<void>(sub_830BCCA8, &out_rResult, this, in_pName);
    }

    inline bool CScene::SetMotion(const char* in_pName)
    {
        return GuestToHostFunction<bool>(sub_830BA760, this, in_pName);
    }

    inline void CScene::SetMotionFrame(float in_MotionFrame)
    {
        m_PrevMotionFrame = in_MotionFrame;
        m_MotionFrame = in_MotionFrame;
    }

    inline void CScene::SetPosition(float in_X, float in_Y)
    {
        GuestToHostFunction<void>(sub_830BB550, this, in_X, in_Y);
    }

    inline void CScene::SetHideFlag(uint32_t in_HideFlag)
    {
        GuestToHostFunction<void>(sub_830BB378, this, in_HideFlag);
    }

    inline void CScene::SetRotation(float in_Angle)
    {
        GuestToHostFunction<void>(sub_830BB5F8, this, in_Angle);
    }

    inline void CScene::SetScale(float in_X, float in_Y)
    {
        GuestToHostFunction<void>(sub_830BB650, this, in_X, in_Y);
    }
}
