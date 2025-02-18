#pragma once

#include <SWA.inl>
#include <Hedgehog/Base/hhObject.h>
#include <boost/smart_ptr/shared_ptr.h>

namespace Hedgehog::Universe
{
    class Message : public Base::CObject
    {
    public:
        be<uint32_t> m_pVftable;
        be<uint32_t> m_SenderActorID;
        boost::shared_ptr<Message> m_spSelf;
    };

    class MessageTypeGet : public Message {};
    class MessageTypeSet : public Message {};
}
