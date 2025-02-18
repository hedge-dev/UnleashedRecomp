#pragma once

#include <api/SWA.h>
#include <user/config.h>

class App
{
public:
    static inline bool s_isInit;
    static inline bool s_isMissingDLC;
    static inline bool s_isLoading;
    static inline bool s_isSaveDataCorrupt;

    static inline SWA::CGameModeStage* s_pGameModeStage;
    static inline SWA::Player::CEvilSonicContext* s_pEvilSonicContext;

    static inline ELanguage s_language;

    static inline double s_deltaTime;

    static void Restart(std::vector<std::string> restartArgs = {});
    static void Exit();
};

