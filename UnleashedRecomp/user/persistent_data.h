#pragma once

#include <user/paths.h>

#define EXT_FILENAME    "EXT-DATA"
#define EXT_SIGNATURE   { 'E', 'X', 'T', ' ' }
#define EXT_VERSION     1

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
    struct ExtHeader
    {
        char Signature[4] EXT_SIGNATURE;
        uint32_t Version{ EXT_VERSION };
        uint32_t HeaderSize{ sizeof(ExtHeader) };
        uint32_t Reserved;
    };

    ExtHeader Header;
    bool DLCFlags[6];

    bool VerifySignature() const;
    bool VerifyVersion() const;
    bool VerifyHeader() const;
};
