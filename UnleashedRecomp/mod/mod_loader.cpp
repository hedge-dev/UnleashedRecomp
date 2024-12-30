#include "mod_loader.h"
#include "ini_file.h"

#include <api/Hedgehog/Base/System/hhAllocator.h>
#include <cpu/guest_stack_var.h>
#include <kernel/function.h>
#include <kernel/heap.h>
#include <xxHashMap.h>

enum class ModType
{
    HMM,
    UMM
};

struct Mod
{
    ModType type{};
    std::vector<std::filesystem::path> includeDirs;
    bool merge = false;
    ankerl::unordered_dense::set<std::filesystem::path> readOnly;
};

static std::vector<Mod> g_mods;

std::filesystem::path ModLoader::ResolvePath(std::string_view path)
{
    if (g_mods.empty())
        return {};

    std::string_view root;

    size_t sepIndex = path.find(":\\");
    if (sepIndex != std::string_view::npos)
    {
        root = path.substr(0, sepIndex);
        path.remove_prefix(sepIndex + 2);
    }

    if (root == "save")
    {
        if (!ModLoader::s_saveFilePath.empty())
        {
            if (path == "SYS-DATA")
                return ModLoader::s_saveFilePath;
            else
                return ModLoader::s_saveFilePath.parent_path() / path;
        }

        return {};
    }

    thread_local xxHashMap<std::filesystem::path> s_pathCache;

    XXH64_hash_t hash = XXH3_64bits(path.data(), path.size());
    auto findResult = s_pathCache.find(hash);
    if (findResult != s_pathCache.end())
        return findResult->second;

    std::string pathStr(path);
    std::replace(pathStr.begin(), pathStr.end(), '\\', '/');
    std::filesystem::path fsPath(std::move(pathStr));

    bool canBeMerged = 
        path.find(".arl") == (path.size() - 4) ||
        path.find(".ar.") == (path.size() - 6) ||
        path.find(".ar") == (path.size() - 3);

    for (auto& mod : g_mods)
    {
        if (mod.type == ModType::UMM && mod.merge && canBeMerged && !mod.readOnly.contains(fsPath))
            continue;

        for (auto& includeDir : mod.includeDirs)
        {
            std::filesystem::path modPath = includeDir / fsPath;
            if (std::filesystem::exists(modPath))
                return s_pathCache.emplace(hash, modPath).first->second;
        }
    }

    return s_pathCache.emplace(hash, std::filesystem::path{}).first->second;
}

std::vector<std::filesystem::path>* ModLoader::GetIncludeDirectories(size_t modIndex)
{
    return modIndex < g_mods.size() ? &g_mods[modIndex].includeDirs : nullptr;
}

void ModLoader::Init()
{
    IniFile configIni;
    if (!configIni.read("cpkredir.ini"))
        return;

    if (configIni.getBool("CPKREDIR", "EnableSaveFileRedirection", false))
    {
        std::string saveFilePathU8 = configIni.getString("CPKREDIR", "SaveFileFallback", std::string());
        if (!saveFilePathU8.empty())
            ModLoader::s_saveFilePath = std::u8string_view((const char8_t*)saveFilePathU8.c_str());
    }

    std::string modsDbIniFilePathU8 = configIni.getString("CPKREDIR", "ModsDbIni", "");
    if (modsDbIniFilePathU8.empty())
        return;

    IniFile modsDbIni;
    if (!modsDbIni.read(std::u8string_view((const char8_t*)modsDbIniFilePathU8.c_str())))
        return;

    bool foundModSaveFilePath = false;

    size_t activeModCount = modsDbIni.get<size_t>("Main", "ActiveModCount", 0);
    for (size_t i = 0; i < activeModCount; ++i)
    {
        std::string modId = modsDbIni.getString("Main", fmt::format("ActiveMod{}", i), "");
        if (modId.empty())
            continue;

        std::string modIniFilePathU8 = modsDbIni.getString("Mods", modId, "");
        if (modIniFilePathU8.empty())
            continue;

        std::filesystem::path modIniFilePath(std::u8string_view((const char8_t*)modIniFilePathU8.c_str()));

        IniFile modIni;
        if (!modIni.read(modIniFilePath))
            continue;

        auto modDirectoryPath = modIniFilePath.parent_path();
        std::string modSaveFilePathU8;

        Mod mod;

        if (modIni.contains("Details") || modIni.contains("Filesystem")) // UMM
        {
            mod.type = ModType::UMM;
            mod.includeDirs.emplace_back(modDirectoryPath);
            mod.merge = modIni.getBool("Details", "Merge", modIni.getBool("Filesystem", "Merge", false));

            std::string readOnly = modIni.getString("Details", "Read-only", modIni.getString("Filesystem", "Read-only", std::string()));
            std::replace(readOnly.begin(), readOnly.end(), '\\', '/');
            std::string_view readOnlySplit = readOnly;

            while (!readOnlySplit.empty())
            {
                size_t index = readOnlySplit.find(',');
                if (index == std::string_view::npos)
                {
                    mod.readOnly.emplace(readOnlySplit);
                    break;
                }

                mod.readOnly.emplace(readOnlySplit.substr(0, index));
                readOnlySplit.remove_prefix(index + 1);
            }

            if (!foundModSaveFilePath)
                modSaveFilePathU8 = modIni.getString("Details", "Save", modIni.getString("Filesystem", "Save", std::string()));
        }
        else // HMM
        {
            mod.type = ModType::HMM;

            size_t includeDirCount = modIni.get<size_t>("Main", "IncludeDirCount", 0);
            for (size_t j = 0; j < includeDirCount; j++)
            {
                std::string includeDirU8 = modIni.getString("Main", fmt::format("IncludeDir{}", j), "");
                if (!includeDirU8.empty())
                    mod.includeDirs.emplace_back(modDirectoryPath / std::u8string_view((const char8_t*)includeDirU8.c_str()));
            }

            if (!foundModSaveFilePath)
                modSaveFilePathU8 = modIni.getString("Main", "SaveFile", std::string());
        }

        if (!mod.includeDirs.empty())
            g_mods.emplace_back(std::move(mod));

        if (!modSaveFilePathU8.empty())
        {
            ModLoader::s_saveFilePath = modDirectoryPath / std::u8string_view((const char8_t*)modSaveFilePathU8.c_str());
            foundModSaveFilePath = true;
        }
    }
}

static constexpr uint32_t LZX_SIGNATURE = 0xFF512EE;

static std::span<uint8_t> decompressLzx(PPCContext& ctx, uint8_t* base, const uint8_t* compressedData, size_t compressedDataSize, be<uint32_t>* scratchSpace)
{
    assert(g_memory.IsInMemoryRange(compressedData));

    bool shouldFreeScratchSpace = false;
    if (scratchSpace == nullptr)
    {
        scratchSpace = reinterpret_cast<be<uint32_t>*>(g_userHeap.Alloc(sizeof(uint32_t) * 2));
        shouldFreeScratchSpace = true;
    }

    // Initialize decompressor
    ctx.r3.u32 = 1;
    ctx.r4.u32 = uint32_t((compressedData + 0xC) - base);
    ctx.r5.u32 = *reinterpret_cast<const be<uint32_t>*>(compressedData + 0x8);
    ctx.r6.u32 = uint32_t(reinterpret_cast<uint8_t*>(scratchSpace) - base);
    sub_831CE1A0(ctx, base);

    uint64_t decompressedDataSize = *reinterpret_cast<const be<uint64_t>*>(compressedData + 0x18);
    uint8_t* decompressedData = reinterpret_cast<uint8_t*>(g_userHeap.Alloc(decompressedDataSize));

    uint32_t blockSize = *reinterpret_cast<const be<uint32_t>*>(compressedData + 0x28);
    size_t decompressedDataOffset = 0;
    size_t compressedDataOffset = 0x30;

    while (decompressedDataOffset < decompressedDataSize)
    {
        size_t decompressedBlockSize = decompressedDataSize - decompressedDataOffset;

        if (decompressedBlockSize > blockSize)
            decompressedBlockSize = blockSize;

        *(scratchSpace + 1) = decompressedBlockSize;

        uint32_t compressedBlockSize = *reinterpret_cast<const be<uint32_t>*>(compressedData + compressedDataOffset);

        // Decompress
        ctx.r3.u32 = *scratchSpace;
        ctx.r4.u32 = uint32_t((decompressedData + decompressedDataOffset) - base);
        ctx.r5.u32 = uint32_t(reinterpret_cast<uint8_t*>(scratchSpace + 1) - base);
        ctx.r6.u32 = uint32_t((compressedData + compressedDataOffset + 0x4) - base);
        ctx.r7.u32 = compressedBlockSize;
        sub_831CE0D0(ctx, base);

        decompressedDataOffset += *(scratchSpace + 1);
        compressedDataOffset += 0x4 + compressedBlockSize;
    }

    // Deinitialize decompressor
    ctx.r3.u32 = *scratchSpace;
    sub_831CE150(ctx, base);

    if (shouldFreeScratchSpace)
        g_userHeap.Free(scratchSpace);

    return { decompressedData, decompressedDataSize };
}

// Hedgehog::Database::CDatabaseLoader::ReadArchiveList
PPC_FUNC_IMPL(__imp__sub_82E0D3E8);
PPC_FUNC(sub_82E0D3E8)
{
    if (g_mods.empty())
    {
        __imp__sub_82E0D3E8(ctx, base);
        return;
    }

    thread_local ankerl::unordered_dense::set<std::string> s_fileNames;
    thread_local std::vector<uint8_t> s_fileData;
    thread_local std::filesystem::path s_tempPath;
    s_fileNames.clear();
    s_fileData.clear();

    auto parseArlFileData = [&](const uint8_t* arlFileData, size_t arlFileSize)
        {
            struct ArlHeader
            {
                uint32_t signature;
                uint32_t splitCount;
            };

            auto* arlHeader = reinterpret_cast<const ArlHeader*>(arlFileData);
            size_t arlHeaderSize = sizeof(ArlHeader) + arlHeader->splitCount * sizeof(uint32_t);
            const uint8_t* arlFileNames = arlFileData + arlHeaderSize;

            while (arlFileNames < arlFileData + arlFileSize)
            {
                uint8_t fileNameSize = *arlFileNames;
                ++arlFileNames;

                s_fileNames.emplace(reinterpret_cast<const char*>(arlFileNames), fileNameSize);

                arlFileNames += fileNameSize;
            }

            return arlHeaderSize;
        };

    auto parseArFileData = [&](const uint8_t* arFileData, size_t arFileSize)
        {
            struct ArEntry
            {
                uint32_t entrySize;
                uint32_t dataSize;
                uint32_t dataOffset;
                uint32_t fileDateLow;
                uint32_t fileDateHigh;
            };

            for (size_t i = 16; i < arFileSize; )
            {
                auto entry = reinterpret_cast<const ArEntry*>(arFileData + i);
                s_fileNames.emplace(reinterpret_cast<const char*>(entry + 1));
                i += entry->entrySize;
            }
        };

    std::u8string_view arlFilePathU8(reinterpret_cast<const char8_t*>(base + PPC_LOAD_U32(ctx.r4.u32)));
    std::filesystem::path arlFilePath;
    std::filesystem::path arFilePath;
    std::filesystem::path appendArlFilePath;

    auto r3 = ctx.r3;
    auto r4 = ctx.r4;
    auto r5 = ctx.r5;
    auto r6 = ctx.r6;

    for (auto& mod : g_mods)
    {
        for (auto& includeDir : mod.includeDirs)
        {
            auto loadFile = [&]<typename TFunction>(const std::filesystem::path& filePath, const TFunction& function)
            {
                if (mod.type == ModType::UMM && mod.readOnly.contains(filePath))
                    return false;

                std::ifstream stream(includeDir / filePath, std::ios::binary);
                if (stream.good())
                {
                    be<uint32_t> signature{};
                    stream.read(reinterpret_cast<char*>(&signature), sizeof(signature));

                    stream.seekg(0, std::ios::end);
                    size_t arlFileSize = stream.tellg();
                    stream.seekg(0, std::ios::beg);

                    if (signature == LZX_SIGNATURE)
                    {
                        void* compressedFileData = g_userHeap.Alloc(arlFileSize);
                        stream.read(reinterpret_cast<char*>(compressedFileData), arlFileSize);

                        auto fileData = decompressLzx(ctx, base, reinterpret_cast<uint8_t*>(compressedFileData), arlFileSize, nullptr);

                        g_userHeap.Free(compressedFileData);

                        function(fileData.data(), fileData.size());

                        g_userHeap.Free(fileData.data());
                    }
                    else
                    {
                        s_fileData.resize(arlFileSize);
                        stream.read(reinterpret_cast<char*>(s_fileData.data()), arlFileSize);
                        stream.close();

                        function(s_fileData.data(), arlFileSize);
                    }

                    return true;
                }

                return false;
            };

            if (mod.type == ModType::UMM)
            {
                if (mod.merge)
                {
                    if (arlFilePath.empty())
                    {
                        arlFilePath = arlFilePathU8;
                        arlFilePath += ".arl";
                    }

                    if (!loadFile(arlFilePath, parseArlFileData))
                    {
                        if (arFilePath.empty())
                        {
                            arFilePath = arlFilePathU8;
                            arFilePath += ".ar";
                        }

                        if (!loadFile(arFilePath, parseArFileData))
                        {
                            for (uint32_t i = 0; ; i++)
                            {
                                s_tempPath = arFilePath;
                                s_tempPath += fmt::format(".{:02}", i);

                                if (!loadFile(s_tempPath, parseArFileData))
                                    break;
                            }
                        }
                    }
                }
            }
            else if (mod.type == ModType::HMM)
            {
                if (appendArlFilePath.empty())
                {
                    if (arlFilePath.empty())
                        arlFilePath = arlFilePathU8;

                    appendArlFilePath = arlFilePath.parent_path();
                    appendArlFilePath /= "+";
                    appendArlFilePath += arlFilePath.filename();
                    appendArlFilePath += ".arl";
                }

                loadFile(appendArlFilePath, parseArlFileData);
            }
        }
    }

    ctx.r3 = r3;
    ctx.r4 = r4;
    ctx.r5 = r5;
    ctx.r6 = r6;

    if (s_fileNames.empty())
    {
        __imp__sub_82E0D3E8(ctx, base);
        return;
    }

    size_t arlHeaderSize = parseArlFileData(base + ctx.r5.u32, ctx.r6.u32);
    size_t arlFileSize = arlHeaderSize;

    for (auto& fileName : s_fileNames)
    {
        arlFileSize += 1;
        arlFileSize += fileName.size();
    }

    uint8_t* newArlFileData = reinterpret_cast<uint8_t*>(g_userHeap.Alloc(arlFileSize));
    memcpy(newArlFileData, base + ctx.r5.u32, arlHeaderSize);

    uint8_t* arlFileNames = newArlFileData + arlHeaderSize;
    for (auto& fileName : s_fileNames)
    {
        *arlFileNames = uint8_t(fileName.size());
        ++arlFileNames;
        memcpy(arlFileNames, fileName.data(), fileName.size());
        arlFileNames += fileName.size();
    }

    ctx.r5.u32 = uint32_t(newArlFileData - base);
    ctx.r6.u32 = uint32_t(arlFileSize);

    __imp__sub_82E0D3E8(ctx, base);

    g_userHeap.Free(newArlFileData);
}

// Hedgehog::Database::SLoadElement::SLoadElement
PPC_FUNC_IMPL(__imp__sub_82E140D8);
PPC_FUNC(sub_82E140D8)
{
    // Store archive name as the pretty name to use it later for append archive loading.
    // This is always set to an empty string for archives, so it should be safe to replace.
    if (!g_mods.empty() && PPC_LOAD_U32(ctx.r5.u32) == 0x8200A621)
        ctx.r5.u32 = ctx.r6.u32;

    __imp__sub_82E140D8(ctx, base);
}

// Hedgehog::Database::CDatabaseLoader::CCreateFromArchive::CreateCallback
PPC_FUNC_IMPL(__imp__sub_82E0B500);
PPC_FUNC(sub_82E0B500)
{
    if (g_mods.empty())
    {
        __imp__sub_82E0B500(ctx, base);
        return;
    }

    auto r3 = ctx.r3; // Callback
    auto r4 = ctx.r4; // Database
    auto r5 = ctx.r5; // Name
    auto r6 = ctx.r6; // Data
    auto r7 = ctx.r7; // Size
    auto r8 = ctx.r8; // Callback data

    std::u8string_view arFilePathU8(reinterpret_cast<const char8_t*>(base + PPC_LOAD_U32(ctx.r5.u32)));
    size_t index = arFilePathU8.find(u8".ar.00");
    if (index == (arFilePathU8.size() - 6))
    {
        arFilePathU8.remove_suffix(3);
    }
    else
    {
        index = arFilePathU8.find(u8".ar");

        if (index != (arFilePathU8.size() - 3) ||
            arFilePathU8.starts_with(u8"tg-") ||
            arFilePathU8.starts_with(u8"gia-") ||
            arFilePathU8.starts_with(u8"gi-texture-"))
        {
            __imp__sub_82E0B500(ctx, base);
            return;
        }
    }

    thread_local std::filesystem::path s_tempFilePath;
    s_tempFilePath.clear();

    std::filesystem::path arFilePath;
    std::filesystem::path appendArFilePath;

    for (auto& mod : g_mods)
    {
        for (auto& includeDir : mod.includeDirs)
        {
            auto loadArchive = [&](const std::filesystem::path& arFilePath)
                {
                    if (mod.type == ModType::UMM && mod.readOnly.contains(arFilePath))
                        return false;

                    std::ifstream stream(includeDir / arFilePath, std::ios::binary);
                    if (stream.good())
                    {
                        stream.seekg(0, std::ios::end);
                        size_t arFileSize = stream.tellg();

                        void* arFileData = g_userHeap.Alloc(arFileSize);
                        stream.seekg(0, std::ios::beg);
                        stream.read(reinterpret_cast<char*>(arFileData), arFileSize);
                        stream.close();

                        auto arFileDataHolder = reinterpret_cast<be<uint32_t>*>(g_userHeap.Alloc(sizeof(uint32_t) * 2));

                        if (*reinterpret_cast<be<uint32_t>*>(arFileData) == LZX_SIGNATURE)
                        {
                            auto fileData = decompressLzx(ctx, base, reinterpret_cast<uint8_t*>(arFileData), arFileSize, arFileDataHolder);
                            
                            g_userHeap.Free(arFileData);

                            arFileData = fileData.data();
                            arFileSize = fileData.size();
                        }

                        arFileDataHolder[0] = g_memory.MapVirtual(arFileData);
                        arFileDataHolder[1] = NULL;

                        ctx.r3 = r3;
                        ctx.r4 = r4;
                        ctx.r5 = r5;
                        ctx.r6.u32 = g_memory.MapVirtual(arFileDataHolder);
                        ctx.r7.u32 = uint32_t(arFileSize);
                        ctx.r8 = r8;

                        __imp__sub_82E0B500(ctx, base);

                        g_userHeap.Free(arFileDataHolder);
                        g_userHeap.Free(arFileData);

                        return true;
                    }

                    return false;
                };

            auto loadArchives = [&](const std::filesystem::path& arFilePath)
                {
                    s_tempFilePath = arFilePath;
                    s_tempFilePath += "l";

                    if (mod.type == ModType::UMM && mod.readOnly.contains(s_tempFilePath))
                        return;

                    std::ifstream stream(includeDir / s_tempFilePath, std::ios::binary);
                    if (stream.good())
                    {
                        // TODO: Should cache this instead of re-opening the file.
                        be<uint32_t> signature{};
                        uint32_t splitCount{};
                        stream.read(reinterpret_cast<char*>(&signature), sizeof(signature));

                        if (signature == LZX_SIGNATURE)
                        {
                            stream.seekg(0, std::ios::end);
                            size_t arlFileSize = stream.tellg();
                            stream.seekg(0, std::ios::beg);

                            void* compressedFileData = g_userHeap.Alloc(arlFileSize);
                            stream.read(reinterpret_cast<char*>(compressedFileData), arlFileSize);
                            stream.close();

                            auto fileData = decompressLzx(ctx, base, reinterpret_cast<uint8_t*>(compressedFileData), arlFileSize, nullptr);

                            g_userHeap.Free(compressedFileData);

                            splitCount = *reinterpret_cast<uint32_t*>(fileData.data() + 0x4);

                            g_userHeap.Free(fileData.data());
                        }
                        else
                        {
                            stream.read(reinterpret_cast<char*>(&splitCount), sizeof(splitCount));
                            stream.close(); 
                        }

                        if (splitCount == 0)
                        {
                            loadArchive(arFilePath);
                        }
                        else
                        {
                            for (uint32_t i = 0; i < splitCount; i++)
                            {
                                s_tempFilePath = arFilePath;
                                s_tempFilePath += fmt::format(".{:02}", i);
                                loadArchive(s_tempFilePath);
                            }
                        }
                    }
                    else if (mod.type == ModType::UMM)
                    {
                        if (!loadArchive(arFilePath))
                        {
                            for (uint32_t i = 0; ; i++)
                            {
                                s_tempFilePath = arFilePath;
                                s_tempFilePath += fmt::format(".{:02}", i);
                                if (!loadArchive(s_tempFilePath))
                                    break;
                            }
                        }
                    }
                };

            if (mod.type == ModType::UMM)
            {
                if (mod.merge)
                {
                    if (arFilePath.empty())
                        arFilePath = arFilePathU8;

                    loadArchives(arFilePath);
                }
            }
            else if (mod.type == ModType::HMM)
            {
                if (appendArFilePath.empty())
                {
                    if (arFilePath.empty())
                        arFilePath = arFilePathU8;

                    appendArFilePath = arFilePath.parent_path();
                    appendArFilePath /= "+";
                    appendArFilePath += arFilePath.filename();
                }

                loadArchives(appendArFilePath);
            }
        }
    }

    ctx.r3 = r3;
    ctx.r4 = r4;
    ctx.r5 = r5;
    ctx.r6 = r6;
    ctx.r7 = r7;
    ctx.r8 = r8;

    __imp__sub_82E0B500(ctx, base);
}
