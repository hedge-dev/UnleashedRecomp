#include "achievement_manager.h"
#include <os/logger.h>
#include <ui/achievement_overlay.h>
#include <user/config.h>

#define NUM_RECORDS sizeof(AchievementManager::Data.Records) / sizeof(AchievementData::AchRecord)

time_t AchievementManager::GetTimestamp(uint16_t id)
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

size_t AchievementManager::GetTotalRecords()
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

bool AchievementManager::IsUnlocked(uint16_t id)
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

void AchievementManager::Unlock(uint16_t id)
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

void AchievementManager::Load()
{
    Data = {};
    Status = EAchStatus::Success;

    auto dataPath = GetDataPath(true);

    if (!std::filesystem::exists(dataPath))
    {
        // Try loading base achievement data as fallback.
        dataPath = GetDataPath(false);

        if (!std::filesystem::exists(dataPath))
            return;
    }

    std::error_code ec;
    auto fileSize = std::filesystem::file_size(dataPath, ec);
    auto dataSize = sizeof(AchievementData);

    if (fileSize != dataSize)
    {
        Status = EAchStatus::BadFileSize;
        return;
    }

    std::ifstream file(dataPath, std::ios::binary);

    if (!file)
    {
        Status = EAchStatus::IOError;
        return;
    }

    AchievementData data{};

    file.read((char*)&data.Signature, sizeof(data.Signature));

    if (!data.VerifySignature())
    {
        Status = EAchStatus::BadSignature;
        file.close();
        return;
    }

    file.read((char*)&data.Version, sizeof(data.Version));

    // TODO: upgrade in future if the version changes.
    if (!data.VerifyVersion())
    {
        Status = EAchStatus::BadVersion;
        file.close();
        return;
    }

    file.seekg(0);
    file.read((char*)&data, sizeof(data));

    if (!data.VerifyChecksum())
    {
        Status = EAchStatus::BadChecksum;
        file.close();
        return;
    }

    file.close();

    memcpy(&Data, &data, dataSize);
}

void AchievementManager::Save(bool ignoreStatus)
{
    if (!ignoreStatus && Status != EAchStatus::Success)
    {
        LOGN_WARNING("Achievement data will not be saved in this session!");
        return;
    }

    LOGN("Saving achievements...");

    std::ofstream file(GetDataPath(true), std::ios::binary);

    if (!file)
    {
        LOGN_ERROR("Failed to write achievement data.");
        return;
    }

    Data.Checksum = Data.CalculateChecksum();

    file.write((const char*)&Data, sizeof(AchievementData));
    file.close();

    Status = EAchStatus::Success;
}
