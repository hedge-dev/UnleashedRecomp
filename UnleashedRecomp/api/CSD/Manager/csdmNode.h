#pragma once

#include "SWA.inl"
#include "CSD/Manager/csdmBase.h"
#include "CSD/Manager/csdmResourceBase.h"
#include "CSD/Manager/csdmNodeObserver.h"
#include "CSD/Manager/csdmSubjectBase.h"

namespace Chao::CSD
{
    struct Node;

    class CNode : public CResourceBase<Node>, SubjectBase<CNodeObserver, CNode>, CBase
    {
    public:
        SWA_INSERT_PADDING(0x4C);

        ~CNode() override = default;

        void SetText(const char* in_pText);
        void SetText(const wchar_t* in_pText);
        Hedgehog::Math::CVector2 GetPosition() const;
        void SetPosition(float in_X, float in_Y);
        void SetHideFlag(uint32_t in_HideFlag);
        void SetRotation(float in_Rotation);
        void SetScale(float in_X, float in_Y);
        void SetPatternIndex(uint32_t in_PatternIndex);
    };
}

#include "CSD/Manager/csdmNode.inl"
