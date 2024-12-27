#include "mod_loader.h"
#include "ini_file.h"
#include "../xxHashMap.h"

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
