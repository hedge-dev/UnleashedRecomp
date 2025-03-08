#pragma once
#include <SWA.inl>
namespace SWA::Sequence::Core
{
    class IManager : public Hedgehog::Base::CSynchronizedObject
    {
    public:
        int test;
    };
    class CManager : public IManager, public Hedgehog::Universe::CMessageActor
    {
    public:
        SWA_INSERT_PADDING(0x6);
        boost::shared_ptr<void> m_Field9c;
        xpointer<void> m_FieldA4;
        boost::shared_ptr<void> m_FieldA8;
        xpointer<void> dwordB0;
        int m_FieldB4;
        int m_FieldB8;
        Hedgehog::Base::CSharedString m_FieldBC;
        Hedgehog::Base::CSharedString m_FieldC0;
        SWA_INSERT_PADDING(0x4C);
        uint8_t m_Field110;
        uint8_t m_Field111;
        uint8_t m_Field112;
        uint8_t m_Field113;
        bool m_Field114;
        bool m_Field115;
    };
    SWA_ASSERT_OFFSETOF(CManager, m_Field9c, 0x9C);
    SWA_ASSERT_OFFSETOF(CManager, m_FieldA4, 0xA4);
    SWA_ASSERT_OFFSETOF(CManager, dwordB0, 0xB0);
    SWA_ASSERT_OFFSETOF(CManager, m_FieldBC, 0xBC);
    SWA_ASSERT_OFFSETOF(CManager, m_Field115, 0x115);
};
