#pragma once

#include <user/paths.h>

#define ACH_FILENAME  "ACH-DATA"
#define ACH_SIGNATURE { 'A', 'C', 'H', ' ' }
#define ACH_VERSION   { 1, 0, 0 }
#define ACH_RECORDS   50

class AchievementData
{
public:
    struct AchVersion
    {
        uint8_t Major;
        uint8_t Minor;
        uint8_t Revision;
        uint8_t Reserved;

        bool operator==(const AchVersion& other) const
        {
            return Major == other.Major &&
                   Minor == other.Minor &&
                   Revision == other.Revision;
        }
    };

#pragma pack(push, 1)
    struct AchRecord
    {
        uint16_t ID;
        time_t Timestamp;
        uint16_t Reserved[3];
    };
#pragma pack(pop)

    char Signature[4] ACH_SIGNATURE;
    AchVersion Version ACH_VERSION;
    uint32_t Checksum;
    uint32_t Reserved;
    AchRecord Records[ACH_RECORDS];

    bool VerifySignature() const;
    bool VerifyVersion() const;
    bool VerifyChecksum();
    uint32_t CalculateChecksum();
};
