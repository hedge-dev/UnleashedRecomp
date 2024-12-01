#pragma once

#include <cstdint>
#include <user/paths.h>

#define ACH_SIGNATURE 'ACH '
#define ACH_VERSION 0x01000000

class AchievementData
{
public:
#pragma pack(push, 1)
    struct Record
    {
        uint16_t ID;
        time_t Timestamp;
        uint16_t Reserved[3];
    };
#pragma pack(pop)

    class Data
    {
    public:
        uint32_t Signature{};
        uint32_t Version{};
        uint32_t Reserved[2];
        Record Records[50];
    };

    inline static Data Data{ ACH_SIGNATURE, ACH_VERSION };

    static std::filesystem::path GetDataPath()
    {
        return GetSavePath() / "ACH-DATA";
    }

    static bool IsUnlocked(uint16_t id);
    static void Unlock(uint16_t id);
    static time_t GetTimestamp(uint16_t id);
    static void Load();
    static void Save();
};
