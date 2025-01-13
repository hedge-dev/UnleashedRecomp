// Referenced from: https://github.com/xenia-canary/xenia-canary/blob/canary_experimental/src/xenia/vfs/devices/xcontent_container_device.cc

/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2023 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#pragma once

#include <filesystem>
#include <map>

#include "virtual_file_system.h"

#include "memory_mapped_file.h"

enum class XContentVolumeType
{
    STFS = 0,
    SVOD = 1,
};

enum class SvodLayoutType 
{
    Unknown = 0x0,
    EnhancedGDF = 0x1,
    XSF = 0x2,
    SingleFile = 0x4,
};

struct XContentFileSystem : VirtualFileSystem
{
    struct File
    {
        size_t size = 0;
        uint32_t blockIndex = 0;
        uint32_t blockCount = 0;
    };

    XContentVolumeType volumeType = XContentVolumeType::STFS;
    SvodLayoutType svodLayoutType = SvodLayoutType::Unknown;
    size_t svodStartDataBlock = 0;
    size_t svodBaseOffset = 0;
    size_t svodMagicOffset = 0;
    std::vector<MemoryMappedFile> mappedFiles;
    uint64_t baseOffset = 0;
    std::map<std::string, File> fileMap;
    std::string name;

    XContentFileSystem(const std::filesystem::path &contentPath);
    bool load(const std::string &path, uint8_t *fileData, size_t fileDataMaxByteCount) const override;
    size_t getSize(const std::string &path) const override;
    bool exists(const std::string &path) const override;
    const std::string &getName() const override;
    bool empty() const;

    static std::unique_ptr<XContentFileSystem> create(const std::filesystem::path &contentPath);
    static bool check(const std::filesystem::path &contentPath);
};
