#pragma once 

#include <cstdint>

struct BlockCompressionDiffPatch
{
    uint32_t destinationOffset;
    uint32_t patchBytesOffset;
    uint32_t patchBytesSize;
};

struct BlockCompressionDiffPatchEntry
{
    uint64_t hash;
    uint32_t patchesOffset;
    uint32_t patchCount;
};

struct BlockCompressionDiffPatchHeader
{
    uint32_t entriesOffset;
    uint32_t entryCount;
};
