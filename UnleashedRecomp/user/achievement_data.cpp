#include "achievement_data.h"
#include <ui/achievement_overlay.h>
#include <user/config.h>

#define NUM_RECORDS sizeof(Data.Records) / sizeof(Record)

time_t AchievementData::GetTimestamp(uint16_t id)
{
    for (int i = 0; i < NUM_RECORDS; i++)
    {
        if (!Data.Records[i].ID)
            break;

        if (Data.Records[i].ID == id)
            return Data.Records[i].Timestamp;
    }

    return 0;
}

int AchievementData::GetTotalRecords()
{
    auto result = 0;

    for (int i = 0; i < NUM_RECORDS; i++)
    {
        if (!Data.Records[i].ID)
            break;

        result++;
    }

    return result;
}

bool AchievementData::IsUnlocked(uint16_t id)
{
    for (int i = 0; i < NUM_RECORDS; i++)
    {
        if (!Data.Records[i].ID)
            break;

        if (Data.Records[i].ID == id)
            return true;
    }

    return false;
}

void AchievementData::Unlock(uint16_t id)
{
    if (IsUnlocked(id))
        return;

    for (int i = 0; i < NUM_RECORDS; i++)
    {
        if (Data.Records[i].ID == 0)
        {
            Data.Records[i].ID = id;
            Data.Records[i].Timestamp = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
            break;
        }
    }

    if (Config::AchievementNotifications)
        AchievementOverlay::Open(id);
}

uint32_t AchievementData::CalculateChecksum()
{
    auto result = 0;

    for (int i = 0; i < NUM_RECORDS; i++)
    {
        auto& record = Data.Records[i];

        for (size_t j = 0; j < sizeof(Record); j++)
            result ^= ((uint8_t*)(&record))[j];
    }

    return result;
}

bool AchievementData::VerifySignature()
{
    char sig[4] = ACH_SIGNATURE;

    return Data.Signature[0] == sig[0] &&
           Data.Signature[1] == sig[1] &&
           Data.Signature[2] == sig[2] &&
           Data.Signature[3] == sig[3];
}

bool AchievementData::VerifyVersion()
{
    return Data.Version == Version ACH_VERSION;
}

bool AchievementData::VerifyChecksum()
{
    return Data.Checksum == CalculateChecksum();
}

void AchievementData::Load()
{
    auto dataPath = GetDataPath();

    if (!std::filesystem::exists(dataPath))
        return;

    std::ifstream file(dataPath, std::ios::binary);

    if (!file)
    {
        printf("[*] ERROR: failed to read achievement data.\n");
        return;
    }

    file.read((char*)&Data.Signature, sizeof(Data.Signature));

    if (!VerifySignature())
    {
        printf("[*] ERROR: invalid achievement data signature.\n");

        char sig[4] = ACH_SIGNATURE;

        Data.Signature[0] = sig[0];
        Data.Signature[1] = sig[1];
        Data.Signature[2] = sig[2];
        Data.Signature[3] = sig[3];

        file.close();

        return;
    }

    file.read((char*)&Data.Version, sizeof(Data.Version));

    if (!VerifyVersion())
    {
        printf("[*] ERROR: unsupported achievement data version.\n");
        Data.Version = ACH_VERSION;
        file.close();
        return;
    }

    file.seekg(0);
    file.read((char*)&Data, sizeof(Data));

    // TODO: display error message to user before wiping data?
    if (!VerifyChecksum())
    {
        printf("[*] ERROR: achievement data checksum mismatch.\n");
        memset(&Data.Records, 0, sizeof(Data.Records));
    }

    file.close();
}

void AchievementData::Save()
{
    std::ofstream file(GetDataPath(), std::ios::binary);

    if (!file)
    {
        printf("[*] ERROR: failed to write achievement data.\n");
        return;
    }

    Data.Checksum = CalculateChecksum();

    file.write((const char*)&Data, sizeof(Data));
    file.close();
}
