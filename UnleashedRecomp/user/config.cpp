#include "config.h"

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
        auto toml = toml::parse_file(configPath.string());

        for (auto def : Config::Definitions)
        {
            def->ReadValue(toml);
#if _DEBUG
            printf("%s (0x%p)\n", def->GetDefinition().c_str(), def->GetValue());
#endif
        }
    }
    catch (toml::parse_error& err)
    {
        printf("[*] Failed to parse configuration: %s\n", err.what());
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
        printf("[*] Failed to write configuration.\n");
    }
}
