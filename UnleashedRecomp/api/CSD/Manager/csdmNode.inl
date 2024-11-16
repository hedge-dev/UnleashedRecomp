#include "SWA.inl"

namespace Chao::CSD
{
    inline void CNode::SetText(const char* in_pText)
    {
        GuestToHostFunction<int>(0x830BF640, this, in_pText);
    }

    inline void CNode::SetText(const wchar_t* in_pText)
    {
        GuestToHostFunction<int>(0x830BF640, this, in_pText);
    }

    inline Hedgehog::Math::CVector2 CNode::GetPosition() const
    {
        Hedgehog::Math::CVector2 position;
        GuestToHostFunction<void>(0x830BF008, this, &position);
        return position;
    }

    inline void CNode::SetPosition(float in_X, float in_Y)
    {
        GuestToHostFunction<int>(0x830BF078, this, in_X, in_Y);
    }

    inline void CNode::SetHideFlag(uint32_t in_HideFlag)
    {
        GuestToHostFunction<int>(0x830BF080, this, in_HideFlag);
    }

    inline void CNode::SetRotation(float in_Rotation)
    {
        GuestToHostFunction<int>(0x830BF088, this, in_Rotation);
    }

    inline void CNode::SetScale(float in_X, float in_Y)
    {
        GuestToHostFunction<int>(0x830BF090, this, in_X, in_Y);
    }

    inline void CNode::SetPatternIndex(uint32_t in_PatternIndex)
    {
        GuestToHostFunction<int>(0x830BF300, this, in_PatternIndex);
    }
}
