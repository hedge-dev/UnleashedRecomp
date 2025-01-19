#pragma once


namespace SWA::PathAnimation
{
    class Entity
    {
    public:
        xpointer<void> m_pVftable;
        SWA_INSERT_PADDING(0x18);
    };


    class Controller
    {
    public:
        xpointer<void> m_pVftable;
        SWA_INSERT_PADDING(0x1C);
        be<float> m_DistanceAlongPath;
    };
    SWA_ASSERT_OFFSETOF(Controller, m_DistanceAlongPath, 0x20);
}
