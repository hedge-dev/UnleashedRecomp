#pragma once

#include <user/config.h>

struct FrameLimiter
{
    std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
    int64_t frame = 0;

    void execute(int64_t fps);
};

class App
{
public:
    static inline bool s_isInit;
    static inline bool s_isMissingDLC;

    static inline ELanguage s_language;

    static inline double s_deltaTime;

    static void Restart(std::vector<std::string> restartArgs = {});
    static void Exit();
};

