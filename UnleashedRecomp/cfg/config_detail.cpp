#include "config.h"
#include "config_detail.h"

// CONFIG_DEFINE
template<typename T>
ConfigDef<T>::ConfigDef(std::string section, std::string name, T defaultValue) : Section(section), Name(name), DefaultValue(defaultValue)
{
    Config::Definitions.emplace_back(this);
}

// CONFIG_DEFINE_ENUM
template<typename T>
ConfigDef<T>::ConfigDef(std::string section, std::string name, T defaultValue, std::unordered_map<std::string, T> enumTemplate)
    : Section(section), Name(name), DefaultValue(defaultValue), EnumTemplate(enumTemplate)
{
    for (const auto& pair : EnumTemplate)
        EnumTemplateReverse[pair.second] = pair.first;

    Config::Definitions.emplace_back(this);
}

// CONFIG_DEFINE_CALLBACK
template<typename T>
ConfigDef<T>::ConfigDef(std::string section, std::string name, T defaultValue, std::function<void(ConfigDef<T>*)> readCallback)
    : Section(section), Name(name), DefaultValue(defaultValue), ReadCallback(readCallback)
{
    Config::Definitions.emplace_back(this);
}
