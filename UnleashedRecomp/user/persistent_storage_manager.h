#pragma once

#include <user/persistent_data.h>

enum class EExtBinStatus
{
    Unknown,
    Success,
    NoFile,
    IOError,
    BadFileSize,
    BadSignature,
    BadVersion
};

class PersistentStorageManager
{
public:
    static inline PersistentData Data{};
    static inline EExtBinStatus BinStatus{ EExtBinStatus::Unknown };

    static std::filesystem::path GetDataPath(bool checkForMods)
    {
        return GetSavePath(checkForMods) / EXT_FILENAME;
    }

    static bool ShouldDisplayDLCMessage(bool setOffendingDLCFlag);
    static bool LoadBinary();
    static bool SaveBinary();
};
