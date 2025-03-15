#include "persistent_storage_manager.h"
#include <install/installer.h>
#include <os/logger.h>
#include <user/paths.h>

bool PersistentStorageManager::ShouldDisplayDLCMessage(bool setOffendingDLCFlag)
{
    auto result = false;

    if (BinStatus != EBinStatus::Success)
        return result;

    static std::unordered_map<EDLCFlag, DLC> flags =
    {
        { EDLCFlag::ApotosAndShamar, DLC::ApotosShamar },
        { EDLCFlag::Spagonia, DLC::Spagonia },
        { EDLCFlag::Chunnan, DLC::Chunnan },
        { EDLCFlag::Mazuri, DLC::Mazuri },
        { EDLCFlag::Holoska, DLC::Holoska },
        { EDLCFlag::EmpireCityAndAdabat, DLC::EmpireCityAdabat }
    };

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

void PersistentStorageManager::LoadBinary()
{
    BinStatus = EBinStatus::Success;

    auto dataPath = GetDataPath(true);

    if (!std::filesystem::exists(dataPath))
    {
        // Try loading base persistent data as fallback.
        dataPath = GetDataPath(false);

        if (!std::filesystem::exists(dataPath))
            return;
    }

    std::error_code ec;
    auto fileSize = std::filesystem::file_size(dataPath, ec);
    auto dataSize = sizeof(PersistentData);

    if (fileSize != dataSize)
    {
        BinStatus = EBinStatus::BadFileSize;
        return;
    }

    std::ifstream file(dataPath, std::ios::binary);

    if (!file)
    {
        BinStatus = EBinStatus::IOError;
        return;
    }

    PersistentData data{};

    file.read((char*)&data.Header.Signature, sizeof(data.Header.Signature));

    if (!data.VerifySignature())
    {
        BinStatus = EBinStatus::BadSignature;
        file.close();
        return;
    }

    file.read((char*)&data.Header.Version, sizeof(data.Header.Version));

    // TODO: upgrade in future if the version changes.
    if (!data.VerifyVersion())
    {
        BinStatus = EBinStatus::BadVersion;
        file.close();
        return;
    }

    file.read((char*)&data.Header.HeaderSize, sizeof(data.Header.HeaderSize));

    if (!data.VerifyHeader())
    {
        BinStatus = EBinStatus::BadHeader;
        file.close();
        return;
    }

    file.seekg(0);
    file.read((char*)&data, sizeof(data));
    file.close();

    memcpy(&Data, &data, dataSize);
}

void PersistentStorageManager::SaveBinary()
{
    LOGN("Saving persistent storage binary...");

    std::ofstream file(GetDataPath(true), std::ios::binary);

    if (!file)
    {
        LOGN_ERROR("Failed to write persistent storage binary.");
        return;
    }

    file.write((const char*)&Data, sizeof(PersistentData));
    file.close();

    BinStatus = EBinStatus::Success;
}
