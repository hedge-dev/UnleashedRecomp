#include "achievement_manager.h"
#include <os/logger.h>
#include <kernel/memory.h>
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

void AchievementManager::UnlockAll()
{
    for (uint16_t i = 24; i <= 83; i++)
    {
        if (i == 30)
            i = 31;

        if (i == 55)
            i = 64;

        AchievementManager::Unlock(i);
    }
}

void AchievementManager::Reset()
{
    Data = {};

    // The first usage of the shoe upgrades get stored within a session persistent boolean flag.
    // This causes issues with popping the achievement for the use of these abilities when the player
    // starts a new save file after they already used them in a session as these bools are never reset
    // unless the game is exited.
    // As a solution we reset these flags whenever the achievement data is being reset too.

    // Lay the Smackdown
    *(bool*)g_memory.Translate(0x833647C5) = false;

    // Wall Crawler
    *(bool*)g_memory.Translate(0x83363004) = false;
    
    // Airdevil
    *(bool*)g_memory.Translate(0x833647BC) = false;
    
    // Hyperdrive
    *(bool*)g_memory.Translate(0x833647C4) = false;
}

bool AchievementManager::LoadBinary()
{
    AchievementManager::Reset();

    BinStatus = EAchBinStatus::Success;

    auto dataPath = GetDataPath(true);

    if (!std::filesystem::exists(dataPath))
    {
        // Try loading base achievement data as fallback.
        dataPath = GetDataPath(false);

        if (!std::filesystem::exists(dataPath))
            return true;
    }

    std::error_code ec;
    auto fileSize = std::filesystem::file_size(dataPath, ec);
    auto dataSize = sizeof(AchievementData);

    if (fileSize != dataSize)
    {
        BinStatus = EAchBinStatus::BadFileSize;
        return false;
    }

    std::ifstream file(dataPath, std::ios::binary);

    if (!file)
    {
        BinStatus = EAchBinStatus::IOError;
        return false;
    }

    AchievementData data{};

    file.read((char*)&data.Signature, sizeof(data.Signature));

    if (!data.VerifySignature())
    {
        BinStatus = EAchBinStatus::BadSignature;
        file.close();
        return false;
    }

    file.read((char*)&data.Version, sizeof(data.Version));

    if (!data.VerifyVersion())
    {
        BinStatus = EAchBinStatus::BadVersion;
        file.close();
        return false;
    }

    file.seekg(0);
    file.read((char*)&data, sizeof(data));

    if (!data.VerifyChecksum())
    {
        BinStatus = EAchBinStatus::BadChecksum;
        file.close();
        return false;
    }

    file.close();

    memcpy(&Data, &data, dataSize);

    return true;
}

bool AchievementManager::SaveBinary(bool ignoreStatus)
{
    if (!ignoreStatus && BinStatus != EAchBinStatus::Success)
    {
        LOGN_WARNING("Achievement data will not be saved in this session!");
        return false;
    }

    LOGN("Saving achievements...");

    std::ofstream file(GetDataPath(true), std::ios::binary);

    if (!file)
    {
        LOGN_ERROR("Failed to write achievement data.");
        return false;
    }

    Data.Checksum = Data.CalculateChecksum();

    file.write((const char*)&Data, sizeof(AchievementData));
    file.close();

    BinStatus = EAchBinStatus::Success;

    return true;
}
