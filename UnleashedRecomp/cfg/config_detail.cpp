#include "config.h"
#include "config_detail.h"

// CONFIG_DEFINE
template<typename T>
ConfigDef<T>::ConfigDef(std::string section, std::string name, T defaultValue) : Section(section), Name(name), DefaultValue(defaultValue)
{
    Config::Definitions.emplace_back(this);
}

// CONFIG_DEFINE_LOCALISED
template<typename T>
ConfigDef<T>::ConfigDef(std::string section, std::string name, CONFIG_LOCALE* nameLocale, CONFIG_LOCALE* descLocale, T defaultValue)
    : Section(section), Name(name), NameLocale(nameLocale), DescLocale(descLocale), DefaultValue(defaultValue)
{
    Config::Definitions.emplace_back(this);
}

// CONFIG_DEFINE_ENUM
template<typename T>
ConfigDef<T>::ConfigDef(std::string section, std::string name, T defaultValue, std::unordered_map<std::string, T>* enumTemplate)
    : Section(section), Name(name), DefaultValue(defaultValue), EnumTemplate(enumTemplate)
{
    for (const auto& pair : *EnumTemplate)
        EnumTemplateReverse[pair.second] = pair.first;

    Config::Definitions.emplace_back(this);
}

// CONFIG_DEFINE_ENUM_LOCALISED
template<typename T>
ConfigDef<T>::ConfigDef(std::string section, std::string name, CONFIG_LOCALE* nameLocale, CONFIG_LOCALE* descLocale, T defaultValue, std::unordered_map<std::string, T>* enumTemplate, CONFIG_ENUM_LOCALE(T)* enumLocale)
    : Section(section), Name(name), NameLocale(nameLocale), DescLocale(descLocale), DefaultValue(defaultValue), EnumTemplate(enumTemplate), EnumLocale(enumLocale)
{
    for (const auto& pair : *EnumTemplate)
        EnumTemplateReverse[pair.second] = pair.first;

    Config::Definitions.emplace_back(this);
}

// CONFIG_DEFINE_CALLBACK
template<typename T>
ConfigDef<T>::ConfigDef(std::string section, std::string name, T defaultValue, std::function<void(ConfigDef<T>*)> callback)
    : Section(section), Name(name), DefaultValue(defaultValue), Callback(callback)
{
    Config::Definitions.emplace_back(this);
}

template<typename T>
std::string ConfigDef<T>::GetDescription() const
{
    if (!DescLocale)
        return "";

    if (!DescLocale->count(Config::Language))
    {
        if (DescLocale->count(ELanguage::English))
        {
            return DescLocale->at(ELanguage::English);
        }
        else
        {
            return "";
        }
    }

    return DescLocale->at(Config::Language);
}

template<typename T>
std::string ConfigDef<T>::GetNameLocalised() const
{
    if (!NameLocale)
        return Name;

    if (!NameLocale->count(Config::Language))
    {
        if (NameLocale->count(ELanguage::English))
        {
            return NameLocale->at(ELanguage::English);
        }
        else
        {
            return Name;
        }
    }

    return NameLocale->at(Config::Language);
}

template<typename T>
std::string ConfigDef<T>::GetValueLocalised() const
{
    auto language = Config::Language;
    CONFIG_ENUM_LOCALE(T)* locale = nullptr;

    if constexpr (std::is_enum_v<T>)
    {
        locale = EnumLocale;
    }
    else if constexpr (std::is_same_v<T, bool>)
    {
        locale = &g_bool_locale;
    }

    if (!locale)
        return ToString(false);

    if (!locale->count(language))
    {
        if (locale->count(ELanguage::English))
        {
            language = ELanguage::English;
        }
        else
        {
            return ToString(false);
        }
    }

    auto strings = locale->at(language);

    if (!strings.count(Value))
        return ToString(false);

    return strings.at(Value);
}
