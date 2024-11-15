namespace Chao::CSD
{
    template<typename T>
    void RCPtr<T>::Attach(T* in_pObject)
    {
        RCPtrAbs::AttachAbs(in_pObject);
    }

    template<typename T>
    T* RCPtr<T>::Get() const
    {
        return static_cast<T*>(RCPtrAbs::GetAbs());
    }

    template<typename T>
    void RCPtr<T>::Set(const RCPtr& in_rOther)
    {
        RCPtrAbs::SetAbs(in_rOther);
    }

    template<typename T>
    T* RCPtr<T>::operator*() const
    {
        return Get();
    }

    template<typename T>
    T* RCPtr<T>::operator->() const
    {
        return Get();
    }

    template<typename T>
    RCPtr<T>& RCPtr<T>::operator=(const RCPtr& in_rOther)
    {
        Set(in_rOther);
        return *this;
    }

    template<typename T>
    RCPtr<T>::operator bool() const
    {
        return m_pObject != nullptr;
    }
}
