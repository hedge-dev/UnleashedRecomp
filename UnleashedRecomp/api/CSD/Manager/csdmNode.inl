#include "SWA.inl"

namespace Chao::CSD
{
    inline CNode::~CNode()
    {
        SWA_VIRTUAL_FUNCTION(void, 0, this);
    }

    inline void CNode::SetText(const char* in_pText)
    {
        GuestToHostFunction<int>(0x830BF640, this, in_pText);
    }

    inline void CNode::SetText(const wchar_t* in_pText)
    {
        GuestToHostFunction<int>(0x830BF640, this, in_pText);
    }

    inline Hedgehog::Math::CVector2* CNode::GetPosition() const
    {
        guest_stack_var<Hedgehog::Math::CVector2> pos;
        GuestToHostFunction<void>(0x830BF008, pos.get(), this);
        return pos.get();
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
