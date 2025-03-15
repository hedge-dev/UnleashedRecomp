#pragma once

#include <user/achievement_data.h>

enum class EAchStatus
{
    Unknown,
    Success,
    IOError,
    BadFileSize,
    BadSignature,
    BadVersion,
    BadChecksum
};

class AchievementManager
{
public:
    static inline AchievementData Data{};
    static inline EAchStatus Status{ EAchStatus::Unknown };

    static std::filesystem::path GetDataPath(bool checkForMods)
    {
        return GetSavePath(checkForMods) / ACH_FILENAME;
    }

    static time_t GetTimestamp(uint16_t id);
    static size_t GetTotalRecords();
    static bool IsUnlocked(uint16_t id);
    static void Unlock(uint16_t id);
    static void UnlockAll();
    static void Reset();
    static void Load();
    static void Save(bool ignoreStatus = false);
};
