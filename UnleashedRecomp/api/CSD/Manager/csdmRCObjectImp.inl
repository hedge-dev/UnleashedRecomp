namespace Chao::CSD
{
    template<typename T>
    void RCPtr<T>::RCObjectImp::Deallocate(void* in_pMemory)
    {
        // delete static_cast<T*>(in_pMemory);
    }

    template<typename T>
    RCPtrAbs::RCObject* RCPtr<T>::CreateRCObject()
    {
        return new RCObjectImp();
    }
}
