#include "achievement_data.h"

#define NUM_RECORDS sizeof(Records) / sizeof(AchRecord)

bool AchievementData::VerifySignature() const
{
    char sig[4] = ACH_SIGNATURE;

    return memcmp(Signature, sig, sizeof(Signature)) == 0;
}

bool AchievementData::VerifyVersion() const
{
    return Version <= ACH_VERSION;
}

bool AchievementData::VerifyChecksum()
{
    return Checksum == CalculateChecksum();
}

uint32_t AchievementData::CalculateChecksum()
{
    auto result = 0;

    for (int i = 0; i < NUM_RECORDS; i++)
    {
        auto& record = Records[i];

        for (size_t j = 0; j < sizeof(AchRecord); j++)
            result ^= ((uint8_t*)(&record))[j];
    }

    return result;
}
