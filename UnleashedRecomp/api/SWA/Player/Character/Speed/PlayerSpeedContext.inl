namespace SWA::Player
{
    inline CPlayerSpeedContext* CPlayerSpeedContext::GetInstance()
    {
        return *(xpointer<CPlayerSpeedContext>*)MmGetHostAddress(0x83362F98);
    }
}
