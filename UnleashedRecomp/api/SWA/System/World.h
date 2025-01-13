#pragma once

#include "SWA.inl"
#include "Hedgehog/Base/Thread/hhSynchronizedObject.h"
#include "Hedgehog/Universe/Engine/hhMessageActor.h"

namespace SWA
{
    class CWorld : public Hedgehog::Base::CSynchronizedObject, public Hedgehog::Universe::CMessageActor
    {
    public:
        class CMember
        {
        public:
            //boost::shared_ptr<Hedgehog::Mirage::CRenderScene> m_spRenderScene;
            //Hedgehog::Base::CSharedString m_Name;
            SWA_INSERT_PADDING(0xC);
            boost::shared_ptr<CCamera> m_spCamera;
            boost::shared_ptr<CCamera> m_spOverrideCamera;
            SWA_INSERT_PADDING(0x64);
        };

        xpointer<CMember> m_pMember;

        boost::shared_ptr<CCamera> GetCamera() const;
    };

    //SWA_ASSERT_OFFSETOF(CWorld::CMember, m_spCamera, 0xC);
    //SWA_ASSERT_SIZEOF(CWorld::CMember, 0x80);
}

#include "World.inl"
