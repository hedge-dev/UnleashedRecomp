#pragma once

#include <SWA.inl>

namespace SWA::Inspire
{
    class CSceneData // : public Hedgehog::Database::CDatabaseData
    {
    public:
        SWA_INSERT_PADDING(0x80);
        Hedgehog::Base::CSharedString m_ResourceName;
    };
}
