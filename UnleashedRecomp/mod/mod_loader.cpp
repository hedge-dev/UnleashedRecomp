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
};

static std::vector<Mod> g_mods;

std::filesystem::path ModLoader::RedirectPath(std::string_view path)
{
    if (g_mods.empty())
        return {};

    thread_local xxHashMap<std::filesystem::path> pathCache;

    size_t sepIndex = path.find(":\\");
    if (sepIndex != std::string_view::npos)
        path.remove_prefix(sepIndex + 2);

    XXH64_hash_t hash = XXH3_64bits(path.data(), path.size());
    auto findResult = pathCache.find(hash);
    if (findResult != pathCache.end())
        return findResult->second;

    for (auto& mod : g_mods)
    {
        // TODO: Need to ignore UMM merge archives here.

        for (auto& includeDir : mod.includeDirs)
        {
            std::filesystem::path modPath = includeDir / path;
            if (std::filesystem::exists(modPath))
                return pathCache.emplace(hash, modPath).first->second;
        }
    }

    return pathCache.emplace(hash, std::filesystem::path{}).first->second;
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

    std::string modsDbIniFilePathU8 = configIni.getString("CPKREDIR", "ModsDbIni", "");
    if (modsDbIniFilePathU8.empty())
        return;

    IniFile modsDbIni;
    if (!modsDbIni.read(std::u8string_view((const char8_t*)modsDbIniFilePathU8.c_str())))
        return;

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

        Mod mod;

        if (modIni.contains("Details") || modIni.contains("Filesystem")) // UMM
        {
            mod.type = ModType::UMM;
            mod.includeDirs.emplace_back(std::move(modDirectoryPath));
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
        }

        if (!mod.includeDirs.empty())
            g_mods.emplace_back(std::move(mod));
    }
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

    const char* arlFilePathU8 = reinterpret_cast<const char*>(base + PPC_LOAD_U32(ctx.r4.u32));
    std::filesystem::path appendArlFilePath;

    thread_local ankerl::unordered_dense::set<std::string> fileNames;
    thread_local std::vector<uint8_t> arlFileData;
    fileNames.clear();
    arlFileData.clear();

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

                fileNames.emplace(reinterpret_cast<const char*>(arlFileNames), fileNameSize);

                arlFileNames += fileNameSize;
            }

            return arlHeaderSize;
        };

    auto parseArlFile = [&](const std::filesystem::path& arlFilePath)
        {
            std::ifstream stream(arlFilePath, std::ios::binary);
            if (stream.good())
            {
                stream.seekg(0, std::ios::end);
                size_t arlFileSize = stream.tellg();
                stream.seekg(0, std::ios::beg);
                arlFileData.resize(arlFileSize);
                stream.read(reinterpret_cast<char*>(arlFileData.data()), arlFileSize);
                stream.close();

                parseArlFileData(arlFileData.data(), arlFileSize);
            }
        };

    for (auto& mod : g_mods)
    {
        // TODO: Handle UMM merge archives!
        for (auto& includeDir : mod.includeDirs)
        {
            if (appendArlFilePath.empty())
            {
                std::filesystem::path arlFilePath(std::u8string_view((const char8_t*)arlFilePathU8));
                appendArlFilePath = arlFilePath.parent_path();
                appendArlFilePath /= "+";
                appendArlFilePath += arlFilePath.filename();
                appendArlFilePath += ".arl";
            }

            parseArlFile(includeDir / appendArlFilePath);
        }
    }

    if (fileNames.empty())
    {
        __imp__sub_82E0D3E8(ctx, base);
        return;
    }

    size_t arlHeaderSize = parseArlFileData(base + ctx.r5.u32, ctx.r6.u32);
    size_t arlFileSize = arlHeaderSize;

    for (auto& fileName : fileNames)
    {
        arlFileSize += 1;
        arlFileSize += fileName.size();
    }

    uint8_t* newArlFileData = reinterpret_cast<uint8_t*>(g_userHeap.Alloc(arlFileSize));
    memcpy(newArlFileData, base + ctx.r5.u32, arlHeaderSize);

    uint8_t* arlFileNames = newArlFileData + arlHeaderSize;
    for (auto& fileName : fileNames)
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
    auto r8 = ctx.r8; // Unknown

    std::u8string_view arFilePathU8(reinterpret_cast<const char8_t*>(base + PPC_LOAD_U32(ctx.r5.u32)));
    size_t index = arFilePathU8.find(u8".ar.00");
    if (index == (arFilePathU8.size() - 6))
    {
        arFilePathU8.remove_suffix(3);
    }
    else
    {
        index = arFilePathU8.find(u8".ar");
        if (index != (arFilePathU8.size() - 3))
        {
            __imp__sub_82E0B500(ctx, base);
            return;
        }
    }

    std::filesystem::path appendArFilePath;

    thread_local std::filesystem::path tempFilePath;
    tempFilePath.clear();

    auto loadArchive = [&](const std::filesystem::path& arFilePath)
        {
            std::ifstream stream(arFilePath, std::ios::binary);
            if (stream.good())
            {
                stream.seekg(0, std::ios::end);
                size_t arFileSize = stream.tellg();

                void* arFileData = __HH_ALLOC(arFileSize);
                guest_stack_var<std::pair<be<uint32_t>, be<uint32_t>>> arFileDataHolder;
                arFileDataHolder->first = g_memory.MapVirtual(arFileData);
                GuestToHostFunction<void>(sub_8241F200, &arFileDataHolder->second, arFileData);

                stream.seekg(0, std::ios::beg);
                stream.read(reinterpret_cast<char*>(arFileData), arFileSize);
                stream.close();

                ctx.r3 = r3;
                ctx.r4 = r4;
                ctx.r5 = r5;
                ctx.r6.u32 = g_memory.MapVirtual(arFileDataHolder.get());
                ctx.r7.u32 = uint32_t(arFileSize);
                ctx.r8 = r8;

                __imp__sub_82E0B500(ctx, base);

                return true;
            }

            return false;
        };

    auto loadArchives = [&](const std::filesystem::path& arFilePath, bool allowNoArl)
        {
            tempFilePath = arFilePath;
            tempFilePath += "l";

            std::ifstream stream(tempFilePath, std::ios::binary);
            if (stream.good())
            {
                // TODO: Should cache this instead of re-opening the file.
                uint32_t splitCount{};
                stream.seekg(4, std::ios::beg);
                stream.read(reinterpret_cast<char*>(&splitCount), sizeof(splitCount));
                stream.close();

                if (splitCount == 0)
                {
                    loadArchive(arFilePath);
                }
                else
                {
                    for (uint32_t i = 0; i < splitCount; i++)
                    {
                        tempFilePath = arFilePath;
                        tempFilePath += fmt::format(".{:02}", i);
                        loadArchive(tempFilePath);
                    }
                }
            }
            else if (allowNoArl)
            {
                if (!loadArchive(arFilePath))
                {
                    for (uint32_t i = 0; ; i++)
                    {
                        tempFilePath = arFilePath;
                        tempFilePath += fmt::format(".{:02}", i);
                        if (!loadArchive(tempFilePath))
                            break;
                    }
                }
            }
        };

    for (auto& mod : g_mods)
    {
        // TODO: Need to check for UMM merge archives here.
        for (auto& includeDir : mod.includeDirs)
        {
            if (appendArFilePath.empty())
            {
                std::filesystem::path arFilePath(arFilePathU8);
                appendArFilePath = arFilePath.parent_path();
                appendArFilePath /= "+";
                appendArFilePath += arFilePath.filename();
            }

            loadArchives(includeDir / appendArFilePath, false);
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
