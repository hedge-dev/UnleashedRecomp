#pragma once

#include <SWA.inl>
#include <SWA/System/ApplicationDocument.h>

namespace SWA
{
    class CApplication : public Hedgehog::Base::CObject
    {
    public:
        class CMember
        {
        public:
            xpointer<CApplicationDocument> m_pApplicationDocument;
        };

        SWA_INSERT_PADDING(0x04); // vftable ptr
        xpointer<CMember> m_pMember;
        SWA_INSERT_PADDING(0x18);
    };
}
