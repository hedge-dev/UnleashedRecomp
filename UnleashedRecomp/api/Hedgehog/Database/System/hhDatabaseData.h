#pragma once

#include "Hedgehog/Base/hhObject.h"

namespace Hedgehog::Database
{
    enum EDatabaseDataFlags : uint8_t
    {
        eDatabaseDataFlags_IsMadeOne = 0x1,
        eDatabaseDataFlags_IsMadeAll = 0x2,
        eDatabaseDataFlags_CreatedFromArchive = 0x4,
        eDatabaseDataFlags_IsMadeMakingOne = 0x8
    };

    class CDatabaseData : public Base::CObject
    {
    public:
        uint8_t m_Flags; // see EDatabaseDataFlags
        SWA_INSERT_PADDING(0x04); // TODO: Base::CSharedString m_TypeAndName;

        virtual ~CDatabaseData() = default;
        virtual bool CheckMadeAll();

        bool IsMadeOne() const;
        void SetMadeOne();

        bool IsMadeAllInternal();
        bool IsMadeAll();
    };
}

#include "Hedgehog/Database/System/hhDatabaseData.inl"
