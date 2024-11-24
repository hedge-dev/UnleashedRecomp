namespace SWA
{
    inline CMovieManager* CMovieManager::GetInstance()
    {
        return *(xpointer<CMovieManager>*)MmGetHostAddress(0x8336758C);
    }
}
