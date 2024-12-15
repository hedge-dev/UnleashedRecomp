#include "config.h"
#include "config_detail.h"
#include <locale/locale.h>

// CONFIG_DEFINE
template<typename T>
ConfigDef<T>::ConfigDef(std::string section, std::string name, T defaultValue) : Section(section), Name(name), DefaultValue(defaultValue)
{
    Config::Definitions.emplace_back(this);
}

// CONFIG_DEFINE_LOCALISED
template<typename T>
ConfigDef<T>::ConfigDef(std::string section, std::string name, CONFIG_LOCALE* locale, T defaultValue)
    : Section(section), Name(name), Locale(locale), DefaultValue(defaultValue)
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
ConfigDef<T>::ConfigDef(std::string section, std::string name, CONFIG_LOCALE* locale, T defaultValue, std::unordered_map<std::string, T>* enumTemplate, CONFIG_ENUM_LOCALE(T)* enumLocale)
    : Section(section), Name(name), Locale(locale), DefaultValue(defaultValue), EnumTemplate(enumTemplate), EnumLocale(enumLocale)
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
std::string ConfigDef<T>::GetNameLocalised() const
{
    if (!Locale)
        return Name;

    if (!Locale->count(Config::Language))
    {
        if (Locale->count(ELanguage::English))
        {
            return std::get<0>(Locale->at(ELanguage::English));
        }
        else
        {
            return Name;
        }
    }

    return std::get<0>(Locale->at(Config::Language));
}

template<typename T>
std::string ConfigDef<T>::GetDescription() const
{
    if (!Locale)
        return "";

    if (!Locale->count(Config::Language))
    {
        if (Locale->count(ELanguage::English))
        {
            return std::get<1>(Locale->at(ELanguage::English));
        }
        else
        {
            return "";
        }
    }

    return std::get<1>(Locale->at(Config::Language));
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
        return Value
            ? Localise("Common_On")
            : Localise("Common_Off");
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

    return std::get<0>(strings.at(Value));
}

template<typename T>
std::string ConfigDef<T>::GetValueDescription() const
{
    auto language = Config::Language;
    CONFIG_ENUM_LOCALE(T)* locale = nullptr;

    if constexpr (std::is_enum_v<T>)
    {
        locale = EnumLocale;
    }
    else if constexpr (std::is_same_v<T, bool>)
    {
        return "";
    }

    if (!locale)
        return "";

    if (!locale->count(language))
    {
        if (locale->count(ELanguage::English))
        {
            language = ELanguage::English;
        }
        else
        {
            return "";
        }
    }

    auto strings = locale->at(language);

    if (!strings.count(Value))
        return "";

    return std::get<1>(strings.at(Value));
}

template<typename T>
inline void ConfigDef<T>::GetLocaleStrings(std::vector<std::string_view>& localeStrings) const
{
    if (Locale != nullptr)
    {
        for (auto& [language, nameAndDesc] : *Locale)
        {
            localeStrings.push_back(std::get<0>(nameAndDesc));
            localeStrings.push_back(std::get<1>(nameAndDesc));
        }
    }

    if (EnumLocale != nullptr)
    {
        for (auto& [language, locale] : *EnumLocale)
        {
            for (auto& [value, nameAndDesc] : locale)
            {
                localeStrings.push_back(std::get<0>(nameAndDesc));
                localeStrings.push_back(std::get<1>(nameAndDesc));
            }
        }
    }
}
