#include "achievement_data.h"
#include <fstream>
#include <ui/achievement_overlay.h>
#include <user/config.h>

bool AchievementData::IsUnlocked(uint16_t id)
{
    for (int i = 0; i < sizeof(Data.Records); i++)
    {
        if (Data.Records[i].ID == id)
            return true;
    }

    return false;
}

void AchievementData::Unlock(uint16_t id)
{
    if (IsUnlocked(id))
        return;

    for (int i = 0; i < sizeof(Data.Records); i++)
    {
        if (Data.Records[i].ID == 0)
        {
            Data.Records[i].ID = id;
            break;
        }
    }

    if (Config::AchievementNotifications)
        AchievementOverlay::Open(id);
}

void AchievementData::Load()
{
    auto dataPath = GetDataPath();

    if (!std::filesystem::exists(dataPath))
        return;

    std::ifstream file(dataPath, std::ios::binary);

    if (!file)
    {
        printf("[*] Failed to parse achievement data.\n");
        return;
    }

    file.read((char*)&Data, sizeof(Data));
    file.close();
}

void AchievementData::Save()
{
    std::ofstream file(GetDataPath(), std::ios::binary);

    if (!file)
    {
        printf("[*] Failed to write achievement data.\n");
        return;
    }

    file.write((const char*)&Data, sizeof(Data));
    file.close();
}
