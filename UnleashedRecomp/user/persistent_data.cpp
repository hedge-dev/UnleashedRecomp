#include "persistent_data.h"

bool PersistentData::VerifySignature() const
{
    char sig[4] = EXT_SIGNATURE;

    return memcmp(Header.Signature, sig, sizeof(Header.Signature)) == 0;
}

bool PersistentData::VerifyVersion() const
{
    return Header.Version <= EXT_VERSION;
}

bool PersistentData::VerifyHeader() const
{
    return Header.HeaderSize == sizeof(ExtHeader);
}
