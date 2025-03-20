#pragma once

#include <user/persistent_data.h>

enum class EBinStatus
{
    Unknown,
    Success,
    IOError,
    BadFileSize,
    BadSignature,
    BadVersion,
    BadHeader
};

class PersistentStorageManager
{
public:
    static inline PersistentData Data{};
    static inline EBinStatus BinStatus{ EBinStatus::Unknown };

    static std::filesystem::path GetDataPath(bool checkForMods)
    {
        return GetSavePath(checkForMods) / EXT_FILENAME;
    }

    static bool ShouldDisplayDLCMessage(bool setOffendingDLCFlag);
    static bool LoadBinary();
    static bool SaveBinary();
};
