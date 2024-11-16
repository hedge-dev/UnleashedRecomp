#pragma once

#include "boost/smart_ptr/shared_ptr.h"

namespace Hedgehog::Database
{
    class CDatabase;
}

namespace SWA
{
    class CCsdProject;

    class CCsdDatabaseWrapper
    {
    public:
        Hedgehog::Database::CDatabase* m_pDatabase;

        CCsdDatabaseWrapper(Hedgehog::Database::CDatabase* in_pDatabase) : m_pDatabase(in_pDatabase) {}

        boost::shared_ptr<CCsdProject> GetCsdProject(const Hedgehog::Base::CSharedString& in_rName);
    };
}

#include "SWA/CSD/CsdDatabaseWrapper.inl"
