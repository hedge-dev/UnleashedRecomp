// Referenced from: https://github.com/xenia-canary/xenia-canary/blob/canary_experimental/src/xenia/cpu/xex_module.cc

/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2023 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#include "xex_patcher.h"

#include <bit>
#include <cassert>

#include <aes.hpp>
#include <lzx.h>
#include <mspack.h>
#include <TinySHA1.hpp>

#include "memory_mapped_file.h"

enum Xex2ModuleFlags
{
    XEX_MODULE_MODULE_PATCH = 0x10,
    XEX_MODULE_PATCH_FULL = 0x20,
    XEX_MODULE_PATCH_DELTA = 0x40,
};

enum Xex2HeaderKeys
{
    XEX_HEADER_FILE_FORMAT_INFO = 0x3FF,
    XEX_HEADER_DELTA_PATCH_DESCRIPTOR = 0x5FF,
};

enum Xex2EncryptionType
{
    XEX_ENCRYPTION_NONE = 0,
    XEX_ENCRYPTION_NORMAL = 1,
};

enum Xex2CompressionType
{
    XEX_COMPRESSION_NONE = 0,
    XEX_COMPRESSION_BASIC = 1,
    XEX_COMPRESSION_NORMAL = 2,
    XEX_COMPRESSION_DELTA = 3,
};

enum Xex2SectionType
{
    XEX_SECTION_CODE = 1,
    XEX_SECTION_DATA = 2,
    XEX_SECTION_READONLY_DATA = 3,
};

struct Xex2OptHeader
{
    be<uint32_t> key;

    union
    {
        be<uint32_t> value;
        be<uint32_t> offset;
    };
};

struct Xex2Header
{
    be<uint32_t> magic;
    be<uint32_t> moduleFlags;
    be<uint32_t> headerSize;
    be<uint32_t> reserved;
    be<uint32_t> securityOffset;
    be<uint32_t> headerCount;
    Xex2OptHeader headers[1];
};

struct Xex2PageDescriptor
{
    union
    {
        // Must be endian-swapped before reading the bitfield.
        uint32_t beValue;
        struct
        {
            uint32_t info : 4;
            uint32_t pageCount : 28;
        };
    };

    char dataDigest[0x14];
};

struct Xex2SecurityInfo
{
    be<uint32_t> headerSize;
    be<uint32_t> imageSize;
    char rsaSignature[0x100];
    be<uint32_t> unknown;
    be<uint32_t> imageFlags;
    be<uint32_t> loadAddress;
    char sectionDigest[0x14];
    be<uint32_t> importTableCount;
    char importTableDigest[0x14];
    char xgd2MediaId[0x10];
    char aesKey[0x10];
    be<uint32_t> exportTable;
    char headerDigest[0x14];
    be<uint32_t> region;
    be<uint32_t> allowedMediaTypes;
    be<uint32_t> pageDescriptorCount;
    Xex2PageDescriptor pageDescriptors[1];
};

struct Xex2DeltaPatch
{
    be<uint32_t> oldAddress;
    be<uint32_t> newAddress;
    be<uint16_t> uncompressedLength;
    be<uint16_t> compressedLength;
    char patchData[1];
};

struct Xex2OptDeltaPatchDescriptor
{
    be<uint32_t> size;
    be<uint32_t> targetVersionValue;
    be<uint32_t> sourceVersionValue;
    uint8_t digestSource[0x14];
    uint8_t imageKeySource[0x10];
    be<uint32_t> sizeOfTargetHeaders;
    be<uint32_t> deltaHeadersSourceOffset;
    be<uint32_t> deltaHeadersSourceSize;
    be<uint32_t> deltaHeadersTargetOffset;
    be<uint32_t> deltaImageSourceOffset;
    be<uint32_t> deltaImageSourceSize;
    be<uint32_t> deltaImageTargetOffset;
    Xex2DeltaPatch info;
};

struct Xex2FileBasicCompressionBlock
{
    be<uint32_t> dataSize;
    be<uint32_t> zeroSize;
};

struct Xex2FileBasicCompressionInfo
{
    Xex2FileBasicCompressionBlock firstBlock;
};

struct Xex2CompressedBlockInfo
{
    be<uint32_t> blockSize;
    uint8_t blockHash[20];
};

struct Xex2FileNormalCompressionInfo
{
    be<uint32_t> windowSize;
    Xex2CompressedBlockInfo firstBlock;
};

struct Xex2OptFileFormatInfo
{
    be<uint32_t> infoSize;
    be<uint16_t> encryptionType;
    be<uint16_t> compressionType;
    union
    {
        Xex2FileBasicCompressionInfo basic;
        Xex2FileNormalCompressionInfo normal;
    } compressionInfo;
};

static const void *getOptHeaderPtr(std::span<const uint8_t> moduleBytes, uint32_t headerKey)
{
    if ((headerKey & 0xFF) == 0)
    {
        assert(false && "Wrong type of method for this key. Expected return value is a number.");
        return nullptr;
    }

    const Xex2Header *xex2Header = (const Xex2Header *)(moduleBytes.data());
    for (uint32_t i = 0; i < xex2Header->headerCount; i++)
    {
        const Xex2OptHeader &optHeader = xex2Header->headers[i];
        if (optHeader.key == headerKey)
        {
            if ((headerKey & 0xFF) == 1)
            {
                return &optHeader.value;
            }
            else
            {
                return &moduleBytes.data()[optHeader.offset];
            }
        }
    }

    return nullptr;
}

struct mspack_memory_file
{
    mspack_system sys;
    void *buffer;
    size_t bufferSize;
    size_t offset;
};

static mspack_memory_file *mspack_memory_open(mspack_system *sys, void *buffer, size_t bufferSize)
{
    assert(bufferSize < INT_MAX);

    if (bufferSize >= INT_MAX)
    {
        return nullptr;
    }

    mspack_memory_file *memoryFile = (mspack_memory_file *)(std::calloc(1, sizeof(mspack_memory_file)));
    if (memoryFile == nullptr)
    {
        return memoryFile;
    }

    memoryFile->buffer = buffer;
    memoryFile->bufferSize = bufferSize;
    memoryFile->offset = 0;
    return memoryFile;
}

static void mspack_memory_close(mspack_memory_file *file)
{
    std::free(file);
}

static int mspack_memory_read(mspack_file *file, void *buffer, int chars)
{
    mspack_memory_file *memoryFile = (mspack_memory_file *)(file);
    const size_t remaining = memoryFile->bufferSize - memoryFile->offset;
    const size_t total = std::min(size_t(chars), remaining);
    std::memcpy(buffer, (uint8_t *)(memoryFile->buffer) + memoryFile->offset, total);
    memoryFile->offset += total;
    return int(total);
}

static int mspack_memory_write(mspack_file *file, void *buffer, int chars)
{
    mspack_memory_file *memoryFile = (mspack_memory_file *)(file);
    const size_t remaining = memoryFile->bufferSize - memoryFile->offset;
    const size_t total = std::min(size_t(chars), remaining);
    std::memcpy((uint8_t *)(memoryFile->buffer) + memoryFile->offset, buffer, total);
    memoryFile->offset += total;
    return int(total);
}

static void *mspack_memory_alloc(mspack_system *sys, size_t chars)
{
    return std::calloc(chars, 1);
}

static void mspack_memory_free(void *ptr)
{
    std::free(ptr);
}

static void mspack_memory_copy(void *src, void *dest, size_t chars)
{
    std::memcpy(dest, src, chars);
}

static mspack_system *mspack_memory_sys_create()
{
    auto sys = (mspack_system *)(std::calloc(1, sizeof(mspack_system)));
    if (!sys)
    {
        return nullptr;
    }

    sys->read = mspack_memory_read;
    sys->write = mspack_memory_write;
    sys->alloc = mspack_memory_alloc;
    sys->free = mspack_memory_free;
    sys->copy = mspack_memory_copy;
    return sys;
}

static void mspack_memory_sys_destroy(struct mspack_system *sys)
{
    free(sys);
}

#if defined(_WIN32)
inline bool bitScanForward(uint32_t v, uint32_t *outFirstSetIndex)
{
    return _BitScanForward((unsigned long *)(outFirstSetIndex), v) != 0;
}

inline bool bitScanForward(uint64_t v, uint32_t *outFirstSetIndex)
{
    return _BitScanForward64((unsigned long *)(outFirstSetIndex), v) != 0;
}

#else
inline bool bitScanForward(uint32_t v, uint32_t *outFirstSetIndex)
{
    int i = ffs(v);
    *outFirstSetIndex = i - 1;
    return i != 0;
}

inline bool bitScanForward(uint64_t v, uint32_t *outFirstSetIndex)
{
    int i = __builtin_ffsll(v);
    *outFirstSetIndex = i - 1;
    return i != 0;
}
#endif

static int lzxDecompress(const void *lzxData, size_t lzxLength, void *dst, size_t dstLength, uint32_t windowSize, void *windowData, size_t windowDataLength)
{
    int resultCode = 1;
    uint32_t windowBits;
    if (!bitScanForward(windowSize, &windowBits)) {
        return resultCode;
    }

    mspack_system *sys = mspack_memory_sys_create();
    mspack_memory_file *lzxSrc = mspack_memory_open(sys, (void *)(lzxData), lzxLength);
    mspack_memory_file *lzxDst = mspack_memory_open(sys, dst, dstLength);
    lzxd_stream *lzxd = lzxd_init(sys, (mspack_file *)(lzxSrc), (mspack_file *)(lzxDst), windowBits, 0, 0x8000, dstLength, 0);
    if (lzxd != nullptr) {
        if (windowData != nullptr) {
            size_t paddingLength = windowSize - windowDataLength;
            std::memset(&lzxd->window[0], 0, paddingLength);
            std::memcpy(&lzxd->window[paddingLength], windowData, windowDataLength);
            lzxd->ref_data_size = windowSize;
        }

        resultCode = lzxd_decompress(lzxd, dstLength);
        lzxd_free(lzxd);
    }

    if (lzxSrc) {
        mspack_memory_close(lzxSrc);
    }

    if (lzxDst) {
        mspack_memory_close(lzxDst);
    }

    if (sys) {
        mspack_memory_sys_destroy(sys);
    }

    return resultCode;
}

static int lzxDeltaApplyPatch(const Xex2DeltaPatch *deltaPatch, uint32_t patchLength, uint32_t windowSize, uint8_t *dstData)
{
    const void *patchEnd = (const uint8_t *)(deltaPatch) + patchLength;
    const Xex2DeltaPatch *curPatch = deltaPatch;
    while (patchEnd > curPatch)
    {
        int patchSize = -4; 
        if (curPatch->compressedLength == 0 && curPatch->uncompressedLength == 0 && curPatch->newAddress == 0 && curPatch->oldAddress == 0)
        {
            // End of patch.
            break;
        }

        switch (curPatch->compressedLength)
        {
        case 0:
            // Set the data to zeroes.
            std::memset(&dstData[curPatch->newAddress], 0, curPatch->uncompressedLength);
            break;
        case 1:
            // Move the data.
            std::memcpy(&dstData[curPatch->newAddress], &dstData[curPatch->oldAddress], curPatch->uncompressedLength);
            break;
        default:
            // Decompress the data into the destination.
            patchSize = curPatch->compressedLength - 4;
            int result = lzxDecompress(curPatch->patchData, curPatch->compressedLength, &dstData[curPatch->newAddress], curPatch->uncompressedLength, windowSize, &dstData[curPatch->oldAddress], curPatch->uncompressedLength);
            if (result != 0)
            {
                return result;
            }

            break;
        }

        curPatch++;
        curPatch = (const Xex2DeltaPatch *)((const uint8_t *)(curPatch) + patchSize);
    }

    return 0;
}

XexPatcher::Result XexPatcher::apply(std::span<const uint8_t> xexBytes, std::span<const uint8_t> patchBytes, std::vector<uint8_t> &outBytes, bool skipData)
{
    // Validate headers.
    static const char Xex2Magic[] = "XEX2";
    const Xex2Header *xexHeader = (const Xex2Header *)(xexBytes.data());
    if (memcmp(xexBytes.data(), Xex2Magic, 4) != 0)
    {
        return Result::XexFileInvalid;
    }

    const Xex2Header *patchHeader = (const Xex2Header *)(patchBytes.data());
    if (memcmp(patchBytes.data(), Xex2Magic, 4) != 0)
    {
        return Result::PatchFileInvalid;
    }

    if ((patchHeader->moduleFlags & (XEX_MODULE_MODULE_PATCH | XEX_MODULE_PATCH_DELTA | XEX_MODULE_PATCH_FULL)) == 0)
    {
        return Result::PatchFileInvalid;
    }

    // Validate patch.
    const Xex2OptDeltaPatchDescriptor *patchDescriptor = (const Xex2OptDeltaPatchDescriptor *)(getOptHeaderPtr(patchBytes, XEX_HEADER_DELTA_PATCH_DESCRIPTOR));
    if (patchDescriptor == nullptr)
    {
        return Result::PatchFileInvalid;
    }
    
    const Xex2OptFileFormatInfo *patchFileFormatInfo = (const Xex2OptFileFormatInfo *)(getOptHeaderPtr(patchBytes, XEX_HEADER_FILE_FORMAT_INFO));
    if (patchFileFormatInfo == nullptr)
    {
        return Result::PatchFileInvalid;
    }

    if (patchFileFormatInfo->compressionType != XEX_COMPRESSION_DELTA)
    {
        return Result::PatchFileInvalid;
    }

    if (patchDescriptor->deltaHeadersSourceOffset > xexHeader->headerSize)
    {
        return Result::PatchIncompatible;
    }

    if (patchDescriptor->deltaHeadersSourceSize > (xexHeader->headerSize - patchDescriptor->deltaHeadersSourceOffset))
    {
        return Result::PatchIncompatible;
    }

    if (patchDescriptor->deltaHeadersTargetOffset > patchDescriptor->sizeOfTargetHeaders)
    {
        return Result::PatchIncompatible;
    }

    uint32_t deltaTargetSize = patchDescriptor->sizeOfTargetHeaders - patchDescriptor->deltaHeadersTargetOffset;
    if (patchDescriptor->deltaHeadersSourceSize > deltaTargetSize)
    {
        return Result::PatchIncompatible;
    }

    // Apply patch.
    uint32_t headerTargetSize = patchDescriptor->sizeOfTargetHeaders;
    if (headerTargetSize == 0)
    {
        headerTargetSize = patchDescriptor->deltaHeadersTargetOffset + patchDescriptor->deltaHeadersSourceSize;
    }

    // Create the bytes for the new XEX header. Copy over the existing data.
    uint32_t newXexHeaderSize = std::max(headerTargetSize, xexHeader->headerSize.get());
    outBytes.resize(newXexHeaderSize);
    memset(outBytes.data(), 0, newXexHeaderSize);
    memcpy(outBytes.data(), xexBytes.data(), headerTargetSize);

    Xex2Header *newXexHeader = (Xex2Header *)(outBytes.data());
    if (patchDescriptor->deltaHeadersSourceOffset > 0)
    {
        memcpy(&outBytes[patchDescriptor->deltaHeadersTargetOffset], &outBytes[patchDescriptor->deltaHeadersSourceOffset], patchDescriptor->deltaHeadersSourceSize);
    }

    int resultCode = lzxDeltaApplyPatch(&patchDescriptor->info, patchDescriptor->size, patchFileFormatInfo->compressionInfo.normal.windowSize, outBytes.data());
    if (resultCode != 0)
    {
        return Result::PatchFailed;
    }

    // Make the header the specified size by the patch.
    outBytes.resize(headerTargetSize);
    newXexHeader = (Xex2Header *)(outBytes.data());

    // Copy the rest of the data.
    const Xex2SecurityInfo *newSecurityInfo = (const Xex2SecurityInfo *)(&outBytes[newXexHeader->securityOffset]);
    outBytes.resize(outBytes.size() + newSecurityInfo->imageSize);
    memset(&outBytes[headerTargetSize], 0, outBytes.size() - headerTargetSize);
    memcpy(&outBytes[headerTargetSize], &xexBytes[xexHeader->headerSize], xexBytes.size() - xexHeader->headerSize);
    newXexHeader = (Xex2Header *)(outBytes.data());
    newSecurityInfo = (const Xex2SecurityInfo *)(&outBytes[newXexHeader->securityOffset]);
    
    // Decrypt the keys and validate that the patch is compatible with the base file.
    static const uint32_t KeySize = 16;
    static const uint8_t Xex2RetailKey[16] = { 0x20, 0xB1, 0x85, 0xA5, 0x9D, 0x28, 0xFD, 0xC3, 0x40, 0x58, 0x3F, 0xBB, 0x08, 0x96, 0xBF, 0x91 };
    static const uint8_t AESBlankIV[AES_BLOCKLEN] = {};
    const Xex2SecurityInfo *originalSecurityInfo = (const Xex2SecurityInfo *)(&xexBytes[xexHeader->securityOffset]);
    const Xex2SecurityInfo *patchSecurityInfo = (const Xex2SecurityInfo *)(&patchBytes[patchHeader->securityOffset]);
    uint8_t decryptedOriginalKey[KeySize];
    uint8_t decryptedNewKey[KeySize];
    uint8_t decryptedPatchKey[KeySize];
    uint8_t decrpytedImageKeySource[KeySize];
    memcpy(decryptedOriginalKey, originalSecurityInfo->aesKey, KeySize);
    memcpy(decryptedNewKey, newSecurityInfo->aesKey, KeySize);
    memcpy(decryptedPatchKey, patchSecurityInfo->aesKey, KeySize);
    memcpy(decrpytedImageKeySource, patchDescriptor->imageKeySource, KeySize);

    AES_ctx aesContext;
    AES_init_ctx_iv(&aesContext, Xex2RetailKey, AESBlankIV);
    AES_CBC_decrypt_buffer(&aesContext, decryptedOriginalKey, KeySize);

    AES_ctx_set_iv(&aesContext, AESBlankIV);
    AES_CBC_decrypt_buffer(&aesContext, decryptedNewKey, KeySize);

    AES_init_ctx_iv(&aesContext, decryptedNewKey, AESBlankIV);
    AES_CBC_decrypt_buffer(&aesContext, decryptedPatchKey, KeySize);

    AES_ctx_set_iv(&aesContext, AESBlankIV);
    AES_CBC_decrypt_buffer(&aesContext, decrpytedImageKeySource, KeySize);

    // Validate the patch's key matches the one from the original XEX.
    if (memcmp(decrpytedImageKeySource, decryptedOriginalKey, KeySize) != 0)
    {
        return Result::PatchIncompatible;
    }

    // Don't process the rest of the patch.
    if (skipData)
    {
        return Result::Success;
    }
    
    // Decrypt base XEX if necessary.
    const Xex2OptFileFormatInfo *fileFormatInfo = (const Xex2OptFileFormatInfo *)(getOptHeaderPtr(xexBytes, XEX_HEADER_FILE_FORMAT_INFO));
    if (fileFormatInfo == nullptr)
    {
        return Result::XexFileInvalid;
    }

    if (fileFormatInfo->encryptionType == XEX_ENCRYPTION_NORMAL)
    {
        AES_init_ctx_iv(&aesContext, decryptedOriginalKey, AESBlankIV);
        AES_CBC_decrypt_buffer(&aesContext, &outBytes[headerTargetSize], xexBytes.size() - xexHeader->headerSize);
    }
    else if (fileFormatInfo->encryptionType != XEX_ENCRYPTION_NONE)
    {
        return Result::XexFileInvalid;
    }

    // Decompress base XEX if necessary.
    if (fileFormatInfo->compressionType == XEX_COMPRESSION_BASIC)
    {
        const Xex2FileBasicCompressionBlock *blocks = &fileFormatInfo->compressionInfo.basic.firstBlock;
        int32_t numBlocks = (fileFormatInfo->infoSize / sizeof(Xex2FileBasicCompressionBlock)) - 1;
        int32_t baseCompressedSize = 0;
        int32_t baseImageSize = 0;
        for (int32_t i = 0; i < numBlocks; i++) {
            baseCompressedSize += blocks[i].dataSize;
            baseImageSize += blocks[i].dataSize + blocks[i].zeroSize;
        }

        if (outBytes.size() < (headerTargetSize + baseImageSize))
        {
            return Result::XexFileInvalid;
        }
        
        // Reverse iteration allows to perform this decompression in place.
        uint8_t *srcDataCursor = outBytes.data() + headerTargetSize + baseCompressedSize;
        uint8_t *outDataCursor = outBytes.data() + headerTargetSize + baseImageSize;
        for (int32_t i = numBlocks - 1; i >= 0; i--)
        {
            outDataCursor -= blocks[i].zeroSize;
            memset(outDataCursor, 0, blocks[i].zeroSize);
            outDataCursor -= blocks[i].dataSize;
            srcDataCursor -= blocks[i].dataSize;
            memmove(outDataCursor, srcDataCursor, blocks[i].dataSize);
        }
    }
    else if (fileFormatInfo->compressionType == XEX_COMPRESSION_NORMAL || fileFormatInfo->compressionType == XEX_COMPRESSION_DELTA)
    {
        return Result::XexFileUnsupported;
    }
    else if (fileFormatInfo->compressionType != XEX_COMPRESSION_NONE)
    {
        return Result::XexFileInvalid;
    }

    Xex2OptFileFormatInfo *newFileFormatInfo = (Xex2OptFileFormatInfo *)(getOptHeaderPtr(outBytes, XEX_HEADER_FILE_FORMAT_INFO));
    if (newFileFormatInfo == nullptr)
    {
        return Result::PatchFailed;
    }
    
    // Update the header to indicate no encryption or compression is used.
    newFileFormatInfo->encryptionType = XEX_ENCRYPTION_NONE;
    newFileFormatInfo->compressionType = XEX_COMPRESSION_NONE;

    // Copy and decrypt patch data if necessary.
    std::vector<uint8_t> patchData;
    patchData.resize(patchBytes.size() - patchHeader->headerSize);
    memcpy(patchData.data(), &patchBytes[patchHeader->headerSize], patchData.size());

    if (patchFileFormatInfo->encryptionType == XEX_ENCRYPTION_NORMAL)
    {
        AES_init_ctx_iv(&aesContext, decryptedPatchKey, AESBlankIV);
        AES_CBC_decrypt_buffer(&aesContext, patchData.data(), patchData.size());
    }
    else if (patchFileFormatInfo->encryptionType != XEX_ENCRYPTION_NONE)
    {
        return Result::PatchFileInvalid;
    }

    const Xex2CompressedBlockInfo *currentBlock = &patchFileFormatInfo->compressionInfo.normal.firstBlock;
    uint8_t *outExe = &outBytes[newXexHeader->headerSize];
    if (patchDescriptor->deltaImageSourceOffset > 0)
    {
        memcpy(&outExe[patchDescriptor->deltaImageTargetOffset], &outExe[patchDescriptor->deltaImageSourceOffset], patchDescriptor->deltaImageSourceSize);
    }

    static const uint32_t DigestSize = 20;
    uint8_t sha1Digest[DigestSize];
    sha1::SHA1 sha1Context;
    uint8_t *patchDataCursor = patchData.data();
    while (currentBlock->blockSize > 0)
    {
        const Xex2CompressedBlockInfo *nextBlock = (const Xex2CompressedBlockInfo *)(patchDataCursor);

        // Hash and validate the block.
        sha1Context.reset();
        sha1Context.processBytes(patchDataCursor, currentBlock->blockSize);
        sha1Context.finalize(sha1Digest);
        if (memcmp(sha1Digest, currentBlock->blockHash, DigestSize) != 0)
        {
            return Result::PatchFailed;
        }

        patchDataCursor += 24;

        // Apply the block's patch data.
        uint32_t blockDataSize = currentBlock->blockSize - 24;
        if (lzxDeltaApplyPatch((const Xex2DeltaPatch *)(patchDataCursor), blockDataSize, patchFileFormatInfo->compressionInfo.normal.windowSize, outExe) != 0)
        {
            return Result::PatchFailed;
        }

        patchDataCursor += blockDataSize;
        currentBlock = nextBlock;
    }

    return Result::Success;
}

XexPatcher::Result XexPatcher::apply(const std::filesystem::path &baseXexPath, const std::filesystem::path &patchXexPath, const std::filesystem::path &newXexPath)
{
    MemoryMappedFile baseXexFile(baseXexPath);
    MemoryMappedFile patchFile(patchXexPath);
    if (!baseXexFile.isOpen() || !patchFile.isOpen())
    {
        return Result::FileOpenFailed;
    }

    std::vector<uint8_t> newXexBytes;
    Result result = apply({ baseXexFile.data(), baseXexFile.size() }, { patchFile.data(), patchFile.size() }, newXexBytes, false);
    if (result != Result::Success)
    {
        return result;
    }

    std::ofstream newXexFile(newXexPath, std::ios::binary);
    if (!newXexFile.is_open())
    {
        return Result::FileOpenFailed;
    }

    newXexFile.write((const char *)(newXexBytes.data()), newXexBytes.size());
    newXexFile.close();

    if (newXexFile.bad())
    {
        std::filesystem::remove(newXexPath);
        return Result::FileWriteFailed;
    }

    return Result::Success;
}
