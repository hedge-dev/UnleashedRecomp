#pragma once

#include <SWA.inl>

namespace SWA::Message
{
    class MsgPopCameraController : public Hedgehog::Universe::MessageTypeSet
    {
    public:
        SWA_INSERT_PADDING(0x08);
        xpointer<CCameraController> m_pCameraController;
        SWA_INSERT_PADDING(0x08);
        xpointer<Hedgehog::Base::CSharedString> m_pCameraName;
        be<float> m_InterpolateTime;
        bool m_Field2C;
        bool m_Field2D;
        bool m_Field2E;

        MsgPopCameraController(CCameraController* in_pCameraController, float in_interpolateTime)
            : m_pCameraController(in_pCameraController), m_InterpolateTime(in_interpolateTime) {}

        MsgPopCameraController(Hedgehog::Base::CSharedString* in_pCameraName, float in_interpolateTime)
            : m_pCameraName(in_pCameraName), m_InterpolateTime(in_interpolateTime) {}
    };

    SWA_ASSERT_OFFSETOF(MsgPopCameraController, m_pCameraController, 0x18);
    SWA_ASSERT_OFFSETOF(MsgPopCameraController, m_pCameraName, 0x24);
    SWA_ASSERT_OFFSETOF(MsgPopCameraController, m_InterpolateTime, 0x28);
    SWA_ASSERT_OFFSETOF(MsgPopCameraController, m_Field2C, 0x2C);
    SWA_ASSERT_OFFSETOF(MsgPopCameraController, m_Field2D, 0x2D);
    SWA_ASSERT_OFFSETOF(MsgPopCameraController, m_Field2E, 0x2E);
}
