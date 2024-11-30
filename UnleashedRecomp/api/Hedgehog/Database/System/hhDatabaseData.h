#pragma once

#include <Hedgehog/Base/hhObject.h>
#include <Hedgehog/Base/Type/hhSharedString.h>

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
        struct Vftable
        {
            be<uint32_t> m_fpDtor;
            be<uint32_t> m_fpCheckMadeAll;
        };

        xpointer<Vftable> m_pVftable;
        uint8_t m_Flags;
        Base::CSharedString m_TypeAndName;

        ~CDatabaseData();
        bool CheckMadeAll();

        bool IsMadeOne() const;
        void SetMadeOne();

        bool IsMadeAllInternal();
        bool IsMadeAll();
    };
}

#include "Hedgehog/Database/System/hhDatabaseData.inl"
