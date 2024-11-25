#pragma once

#include "SWA.inl"
#include "Hedgehog/Universe/Thread/hhParallelJob.h"

namespace Hedgehog::Universe
{
    class CUpdateUnit : public Base::CObject, public IParallelJob
    {
    public:
        xpointer<void> m_pVftable;
        SWA_INSERT_PADDING(0x20);

        CUpdateUnit(const swa_null_ctor& nil) : CObject(nil), IParallelJob(nil) {}
        CUpdateUnit();

        // TODO: implement virtual functions.
        // virtual ~CUpdateUnit();
        // 
        // virtual void ExecuteParallelJob(const SUpdateInfo& in_rUpdateInfo) override;
        // 
        // virtual void UpdateParallel(const SUpdateInfo& in_rUpdateInfo) {}
        // virtual void UpdateSerial(const SUpdateInfo& in_rUpdateInfo) {}
    };
}
