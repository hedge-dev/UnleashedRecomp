namespace SWA
{
    inline boost::shared_ptr<CCsdProject> CCsdDatabaseWrapper::GetCsdProject(const Hedgehog::Base::CSharedString& in_rName)
    {
        boost::shared_ptr<CCsdProject> spCsdProject;
        GuestToHostFunction<void>(0x825E2B40, &in_rName, this, &spCsdProject, 0);
        return spCsdProject;
    }
}
