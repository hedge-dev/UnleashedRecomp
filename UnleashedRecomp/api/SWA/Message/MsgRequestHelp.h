#pragma once

#include <SWA.inl>

namespace SWA::Message
{
    class MsgRequestHelp
    {
    public:
        SWA_INSERT_PADDING(0x1C);
        Hedgehog::Base::CSharedString m_Name;
    };
}
