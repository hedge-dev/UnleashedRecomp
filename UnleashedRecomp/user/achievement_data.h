#pragma once

#include <user/paths.h>

#define ACH_SIGNATURE { 'A', 'C', 'H', ' ' }
#define ACH_VERSION   { 1, 0, 0 }
#define ACH_RECORDS   50

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

    struct Version
    {
        uint8_t Major;
        uint8_t Minor;
        uint8_t Revision;
        uint8_t Reserved;

        bool operator==(const Version& other) const
        {
            return Major == other.Major &&
                   Minor == other.Minor &&
                   Revision == other.Revision;
        }
    };

    class Data
    {
    public:
        char Signature[4];
        Version Version{};
        uint32_t Checksum;
        uint32_t Reserved;
        Record Records[ACH_RECORDS];
    };

    inline static Data Data{ ACH_SIGNATURE, ACH_VERSION };

    static std::filesystem::path GetDataPath()
    {
        return GetSavePath() / "ACH-DATA";
    }

    static time_t GetTimestamp(uint16_t id);
    static int GetTotalRecords();
    static bool IsUnlocked(uint16_t id);
    static void Unlock(uint16_t id);
    static uint32_t CalculateChecksum();
    static bool VerifySignature();
    static bool VerifyVersion();
    static bool VerifyChecksum();
    static void Load();
    static void Save();
};
