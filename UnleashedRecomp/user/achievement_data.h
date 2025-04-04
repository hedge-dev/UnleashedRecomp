#pragma once

#include <user/paths.h>

#define ACH_FILENAME  "ACH-DATA"
#define ACH_SIGNATURE { 'A', 'C', 'H', ' ' }
#define ACH_VERSION   1
#define ACH_RECORDS   50

class AchievementData
{
public:
#pragma pack(push, 1)
    struct AchRecord
    {
        uint16_t ID;
        time_t Timestamp;
        uint16_t Reserved[3];
    };
#pragma pack(pop)

    char Signature[4] ACH_SIGNATURE;
    uint32_t Version{ ACH_VERSION };
    uint32_t Checksum{};
    uint32_t Reserved{};
    AchRecord Records[ACH_RECORDS]{};

    bool VerifySignature() const;
    bool VerifyVersion() const;
    bool VerifyChecksum();
    uint32_t CalculateChecksum();
};
