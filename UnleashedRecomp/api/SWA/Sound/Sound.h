#pragma once

namespace SWA
{
    class CSound // : public CGameObject
    {
    public:
        struct SSoundInfo
        {
        public:
            SWA_INSERT_PADDING(0x04);
            xpointer<void> m_pAdminSimplePlayer;
            SWA_INSERT_PADDING(0x04);
            xpointer<void> m_pAdminBgm;
        };

        SWA_INSERT_PADDING(0xC4);
        xpointer<SSoundInfo> m_pSoundInfo;
    };
}
