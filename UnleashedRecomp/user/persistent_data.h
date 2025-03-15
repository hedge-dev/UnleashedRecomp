#pragma once

#include <user/paths.h>

#define EXT_FILENAME    "EXT-DATA"
#define EXT_SIGNATURE   { 'E', 'X', 'T', ' ' }
#define EXT_VERSION     { 1, 0, 0 }

enum class EDLCFlag
{
    ApotosAndShamar,
    Spagonia,
    Chunnan,
    Mazuri,
    Holoska,
    EmpireCityAndAdabat,
    Count
};

class PersistentData
{
public:
    struct ExtVersion
    {
        uint8_t Major;
        uint8_t Minor;
        uint8_t Revision;
        uint8_t Reserved;

        bool operator==(const ExtVersion& other) const
        {
            return Major == other.Major &&
                   Minor == other.Minor &&
                   Revision == other.Revision;
        }
    };

    struct ExtHeader
    {
        char Signature[4] EXT_SIGNATURE;
        ExtVersion Version EXT_VERSION;
        uint32_t HeaderSize{ sizeof(ExtHeader) };
        uint32_t Reserved;
    };

    ExtHeader Header;
    bool DLCFlags[6];

    bool VerifySignature() const;
    bool VerifyVersion() const;
    bool VerifyHeader() const;
};
