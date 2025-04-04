#include "persistent_storage_manager.h"
#include <install/installer.h>
#include <os/logger.h>
#include <user/paths.h>

bool PersistentStorageManager::ShouldDisplayDLCMessage(bool setOffendingDLCFlag)
{
    if (BinStatus != EExtBinStatus::Success)
        return true;

    static std::unordered_map<EDLCFlag, DLC> flags =
    {
        { EDLCFlag::ApotosAndShamar, DLC::ApotosShamar },
        { EDLCFlag::Spagonia, DLC::Spagonia },
        { EDLCFlag::Chunnan, DLC::Chunnan },
        { EDLCFlag::Mazuri, DLC::Mazuri },
        { EDLCFlag::Holoska, DLC::Holoska },
        { EDLCFlag::EmpireCityAndAdabat, DLC::EmpireCityAdabat }
    };

    auto result = false;

    for (auto& pair : flags)
    {
        if (!Data.DLCFlags[(int)pair.first] && Installer::checkDLCInstall(GetGamePath(), pair.second))
        {
            if (setOffendingDLCFlag)
                Data.DLCFlags[(int)pair.first] = true;

            result = true;
        }
    }

    return result;
}

bool PersistentStorageManager::LoadBinary()
{
    BinStatus = EExtBinStatus::Success;

    auto dataPath = GetDataPath(true);

    if (!std::filesystem::exists(dataPath))
    {
        // Try loading base persistent data as fallback.
        dataPath = GetDataPath(false);

        if (!std::filesystem::exists(dataPath))
            return true;
    }

    std::error_code ec;
    auto fileSize = std::filesystem::file_size(dataPath, ec);
    auto dataSize = sizeof(PersistentData);

    if (fileSize != dataSize)
    {
        BinStatus = EExtBinStatus::BadFileSize;
        return false;
    }

    std::ifstream file(dataPath, std::ios::binary);

    if (!file)
    {
        BinStatus = EExtBinStatus::IOError;
        return false;
    }

    PersistentData data{};

    file.read((char*)&data.Signature, sizeof(data.Signature));

    if (!data.VerifySignature())
    {
        BinStatus = EExtBinStatus::BadSignature;
        file.close();
        return false;
    }

    file.read((char*)&data.Version, sizeof(data.Version));

    if (!data.VerifyVersion())
    {
        BinStatus = EExtBinStatus::BadVersion;
        file.close();
        return false;
    }

    file.seekg(0);
    file.read((char*)&data, sizeof(data));
    file.close();

    memcpy(&Data, &data, dataSize);

    return true;
}

bool PersistentStorageManager::SaveBinary()
{
    LOGN("Saving persistent storage binary...");

    std::ofstream file(GetDataPath(true), std::ios::binary);

    if (!file)
    {
        LOGN_ERROR("Failed to write persistent storage binary.");
        return false;
    }

    file.write((const char*)&Data, sizeof(PersistentData));
    file.close();

    BinStatus = EExtBinStatus::Success;

    return true;
}
