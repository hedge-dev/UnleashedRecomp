#include "config.h"
#include <os/logger.h>

void Config::Load()
{
    auto configPath = GetConfigPath();

    if (!std::filesystem::exists(configPath))
    {
        Config::Save();
        return;
    }

    try
    {
        toml::parse_result toml;
        std::ifstream tomlStream(configPath);
        if (tomlStream.is_open())
        {
            toml = toml::parse(tomlStream);
        }

        for (auto def : Config::Definitions)
        {
            def->ReadValue(toml);
#if _DEBUG
            LOGFN_UTILITY("{} (0x{:X})", def->GetDefinition().c_str(), (intptr_t)def->GetValue());
#endif
        }
    }
    catch (toml::parse_error& err)
    {
        LOGFN_ERROR("Failed to parse configuration: {}", err.what());
    }
}

void Config::Save()
{
    auto userPath = GetUserPath();

    if (!std::filesystem::exists(userPath))
        std::filesystem::create_directory(userPath);

    std::string result;
    std::string section;

    for (auto def : Config::Definitions)
    {
        auto isFirstSection = section.empty();
        auto isDefWithSection = section != def->GetSection();
        auto tomlDef = def->GetDefinition(isDefWithSection);

        section = def->GetSection();

        // Don't output prefix space for first section.
        if (!isFirstSection && isDefWithSection)
            result += '\n';

        result += tomlDef + '\n';
    }

    std::ofstream out(GetConfigPath());

    if (out.is_open())
    {
        out << result;
        out.close();
    }
    else
    {
        LOGN_ERROR("Failed to write configuration.");
    }
}
