#include "SWA.inl"

namespace Chao::CSD
{
    inline CNode::~CNode()
    {
        SWA_VIRTUAL_FUNCTION(void, 0, this);
    }

    inline void CNode::SetText(const char* in_pText)
    {
        GuestToHostFunction<int>(sub_830BF640, this, in_pText);
    }

    inline void CNode::SetText(const wchar_t* in_pText)
    {
        GuestToHostFunction<int>(sub_830BF640, this, in_pText);
    }

    inline void CNode::GetPosition(Hedgehog::Math::CVector2& out_rResult) const
    {
        GuestToHostFunction<void>(sub_830BF008, &out_rResult, this);
    }

    inline void CNode::SetPosition(float in_X, float in_Y)
    {
        GuestToHostFunction<int>(sub_830BF078, this, in_X, in_Y);
    }

    inline void CNode::SetHideFlag(uint32_t in_HideFlag)
    {
        GuestToHostFunction<int>(sub_830BF080, this, in_HideFlag);
    }

    inline void CNode::SetRotation(float in_Rotation)
    {
        GuestToHostFunction<int>(sub_830BF088, this, in_Rotation);
    }

    inline void CNode::SetScale(float in_X, float in_Y)
    {
        GuestToHostFunction<int>(sub_830BF090, this, in_X, in_Y);
    }

    inline void CNode::SetPatternIndex(uint32_t in_PatternIndex)
    {
        GuestToHostFunction<int>(sub_830BF300, this, in_PatternIndex);
    }
}
