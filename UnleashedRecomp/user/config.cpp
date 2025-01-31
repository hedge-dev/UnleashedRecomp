#include "config.h"
#include <os/logger.h>
#include <user/paths.h>
#include <exports.h>

std::vector<IConfigDef*> g_configDefinitions;

#define CONFIG_DEFINE_ENUM_TEMPLATE(type) \
    static std::unordered_map<std::string, type> g_##type##_template =

CONFIG_DEFINE_ENUM_TEMPLATE(ELanguage)
{
    { "English",  ELanguage::English },
    { "Japanese", ELanguage::Japanese },
    { "German",   ELanguage::German },
    { "French",   ELanguage::French },
    { "Spanish",  ELanguage::Spanish },
    { "Italian",  ELanguage::Italian }
};

CONFIG_DEFINE_ENUM_TEMPLATE(ETimeOfDayTransition)
{
    { "Xbox",        ETimeOfDayTransition::Xbox },
    { "PlayStation", ETimeOfDayTransition::PlayStation }
};

CONFIG_DEFINE_ENUM_TEMPLATE(ECameraRotationMode)
{
    { "Normal",  ECameraRotationMode::Normal },
    { "Reverse", ECameraRotationMode::Reverse },
};

CONFIG_DEFINE_ENUM_TEMPLATE(EControllerIcons)
{
    { "Auto",        EControllerIcons::Auto },
    { "Xbox",        EControllerIcons::Xbox },
    { "PlayStation", EControllerIcons::PlayStation }
};

CONFIG_DEFINE_ENUM_TEMPLATE(SDL_Scancode)
{
    { "UNKNOWN", SDL_SCANCODE_UNKNOWN },
    { "A", SDL_SCANCODE_A },
    { "B", SDL_SCANCODE_B },
    { "C", SDL_SCANCODE_C },
    { "D", SDL_SCANCODE_D },
    { "E", SDL_SCANCODE_E },
    { "F", SDL_SCANCODE_F },
    { "G", SDL_SCANCODE_G },
    { "H", SDL_SCANCODE_H },
    { "I", SDL_SCANCODE_I },
    { "J", SDL_SCANCODE_J },
    { "K", SDL_SCANCODE_K },
    { "L", SDL_SCANCODE_L },
    { "M", SDL_SCANCODE_M },
    { "N", SDL_SCANCODE_N },
    { "O", SDL_SCANCODE_O },
    { "P", SDL_SCANCODE_P },
    { "Q", SDL_SCANCODE_Q },
    { "R", SDL_SCANCODE_R },
    { "S", SDL_SCANCODE_S },
    { "T", SDL_SCANCODE_T },
    { "U", SDL_SCANCODE_U },
    { "V", SDL_SCANCODE_V },
    { "W", SDL_SCANCODE_W },
    { "X", SDL_SCANCODE_X },
    { "Y", SDL_SCANCODE_Y },
    { "Z", SDL_SCANCODE_Z },
    { "1", SDL_SCANCODE_1 },
    { "2", SDL_SCANCODE_2 },
    { "3", SDL_SCANCODE_3 },
    { "4", SDL_SCANCODE_4 },
    { "5", SDL_SCANCODE_5 },
    { "6", SDL_SCANCODE_6 },
    { "7", SDL_SCANCODE_7 },
    { "8", SDL_SCANCODE_8 },
    { "9", SDL_SCANCODE_9 },
    { "0", SDL_SCANCODE_0 },
    { "RETURN", SDL_SCANCODE_RETURN },
    { "ESCAPE", SDL_SCANCODE_ESCAPE },
    { "BACKSPACE", SDL_SCANCODE_BACKSPACE },
    { "TAB", SDL_SCANCODE_TAB },
    { "SPACE", SDL_SCANCODE_SPACE },
    { "MINUS", SDL_SCANCODE_MINUS },
    { "EQUALS", SDL_SCANCODE_EQUALS },
    { "LEFTBRACKET", SDL_SCANCODE_LEFTBRACKET },
    { "RIGHTBRACKET", SDL_SCANCODE_RIGHTBRACKET },
    { "BACKSLASH", SDL_SCANCODE_BACKSLASH },
    { "NONUSHASH", SDL_SCANCODE_NONUSHASH },
    { "SEMICOLON", SDL_SCANCODE_SEMICOLON },
    { "APOSTROPHE", SDL_SCANCODE_APOSTROPHE },
    { "GRAVE", SDL_SCANCODE_GRAVE },
    { "COMMA", SDL_SCANCODE_COMMA },
    { "PERIOD", SDL_SCANCODE_PERIOD },
    { "SLASH", SDL_SCANCODE_SLASH },
    { "CAPSLOCK", SDL_SCANCODE_CAPSLOCK },
    { "F1", SDL_SCANCODE_F1 },
    { "F2", SDL_SCANCODE_F2 },
    { "F3", SDL_SCANCODE_F3 },
    { "F4", SDL_SCANCODE_F4 },
    { "F5", SDL_SCANCODE_F5 },
    { "F6", SDL_SCANCODE_F6 },
    { "F7", SDL_SCANCODE_F7 },
    { "F8", SDL_SCANCODE_F8 },
    { "F9", SDL_SCANCODE_F9 },
    { "F10", SDL_SCANCODE_F10 },
    { "F11", SDL_SCANCODE_F11 },
    { "F12", SDL_SCANCODE_F12 },
    { "PRINTSCREEN", SDL_SCANCODE_PRINTSCREEN },
    { "SCROLLLOCK", SDL_SCANCODE_SCROLLLOCK },
    { "PAUSE", SDL_SCANCODE_PAUSE },
    { "INSERT", SDL_SCANCODE_INSERT },
    { "HOME", SDL_SCANCODE_HOME },
    { "PAGEUP", SDL_SCANCODE_PAGEUP },
    { "DELETE", SDL_SCANCODE_DELETE },
    { "END", SDL_SCANCODE_END },
    { "PAGEDOWN", SDL_SCANCODE_PAGEDOWN },
    { "RIGHT", SDL_SCANCODE_RIGHT },
    { "LEFT", SDL_SCANCODE_LEFT },
    { "DOWN", SDL_SCANCODE_DOWN },
    { "UP", SDL_SCANCODE_UP },
    { "NUMLOCKCLEAR", SDL_SCANCODE_NUMLOCKCLEAR },
    { "KP_DIVIDE", SDL_SCANCODE_KP_DIVIDE },
    { "KP_MULTIPLY", SDL_SCANCODE_KP_MULTIPLY },
    { "KP_MINUS", SDL_SCANCODE_KP_MINUS },
    { "KP_PLUS", SDL_SCANCODE_KP_PLUS },
    { "KP_ENTER", SDL_SCANCODE_KP_ENTER },
    { "KP_1", SDL_SCANCODE_KP_1 },
    { "KP_2", SDL_SCANCODE_KP_2 },
    { "KP_3", SDL_SCANCODE_KP_3 },
    { "KP_4", SDL_SCANCODE_KP_4 },
    { "KP_5", SDL_SCANCODE_KP_5 },
    { "KP_6", SDL_SCANCODE_KP_6 },
    { "KP_7", SDL_SCANCODE_KP_7 },
    { "KP_8", SDL_SCANCODE_KP_8 },
    { "KP_9", SDL_SCANCODE_KP_9 },
    { "KP_0", SDL_SCANCODE_KP_0 },
    { "KP_PERIOD", SDL_SCANCODE_KP_PERIOD },
    { "NONUSBACKSLASH", SDL_SCANCODE_NONUSBACKSLASH },
    { "APPLICATION", SDL_SCANCODE_APPLICATION },
    { "POWER", SDL_SCANCODE_POWER },
    { "KP_EQUALS", SDL_SCANCODE_KP_EQUALS },
    { "F13", SDL_SCANCODE_F13 },
    { "F14", SDL_SCANCODE_F14 },
    { "F15", SDL_SCANCODE_F15 },
    { "F16", SDL_SCANCODE_F16 },
    { "F17", SDL_SCANCODE_F17 },
    { "F18", SDL_SCANCODE_F18 },
    { "F19", SDL_SCANCODE_F19 },
    { "F20", SDL_SCANCODE_F20 },
    { "F21", SDL_SCANCODE_F21 },
    { "F22", SDL_SCANCODE_F22 },
    { "F23", SDL_SCANCODE_F23 },
    { "F24", SDL_SCANCODE_F24 },
    { "EXECUTE", SDL_SCANCODE_EXECUTE },
    { "HELP", SDL_SCANCODE_HELP },
    { "MENU", SDL_SCANCODE_MENU },
    { "SELECT", SDL_SCANCODE_SELECT },
    { "STOP", SDL_SCANCODE_STOP },
    { "AGAIN", SDL_SCANCODE_AGAIN },
    { "UNDO", SDL_SCANCODE_UNDO },
    { "CUT", SDL_SCANCODE_CUT },
    { "COPY", SDL_SCANCODE_COPY },
    { "PASTE", SDL_SCANCODE_PASTE },
    { "FIND", SDL_SCANCODE_FIND },
    { "MUTE", SDL_SCANCODE_MUTE },
    { "VOLUMEUP", SDL_SCANCODE_VOLUMEUP },
    { "VOLUMEDOWN", SDL_SCANCODE_VOLUMEDOWN },
    { "KP_COMMA", SDL_SCANCODE_KP_COMMA },
    { "KP_EQUALSAS400", SDL_SCANCODE_KP_EQUALSAS400 },
    { "INTERNATIONAL1", SDL_SCANCODE_INTERNATIONAL1 },
    { "INTERNATIONAL2", SDL_SCANCODE_INTERNATIONAL2 },
    { "INTERNATIONAL3", SDL_SCANCODE_INTERNATIONAL3 },
    { "INTERNATIONAL4", SDL_SCANCODE_INTERNATIONAL4 },
    { "INTERNATIONAL5", SDL_SCANCODE_INTERNATIONAL5 },
    { "INTERNATIONAL6", SDL_SCANCODE_INTERNATIONAL6 },
    { "INTERNATIONAL7", SDL_SCANCODE_INTERNATIONAL7 },
    { "INTERNATIONAL8", SDL_SCANCODE_INTERNATIONAL8 },
    { "INTERNATIONAL9", SDL_SCANCODE_INTERNATIONAL9 },
    { "LANG1", SDL_SCANCODE_LANG1 },
    { "LANG2", SDL_SCANCODE_LANG2 },
    { "LANG3", SDL_SCANCODE_LANG3 },
    { "LANG4", SDL_SCANCODE_LANG4 },
    { "LANG5", SDL_SCANCODE_LANG5 },
    { "LANG6", SDL_SCANCODE_LANG6 },
    { "LANG7", SDL_SCANCODE_LANG7 },
    { "LANG8", SDL_SCANCODE_LANG8 },
    { "LANG9", SDL_SCANCODE_LANG9 },
    { "ALTERASE", SDL_SCANCODE_ALTERASE },
    { "SYSREQ", SDL_SCANCODE_SYSREQ },
    { "CANCEL", SDL_SCANCODE_CANCEL },
    { "CLEAR", SDL_SCANCODE_CLEAR },
    { "PRIOR", SDL_SCANCODE_PRIOR },
    { "RETURN2", SDL_SCANCODE_RETURN2 },
    { "SEPARATOR", SDL_SCANCODE_SEPARATOR },
    { "OUT", SDL_SCANCODE_OUT },
    { "OPER", SDL_SCANCODE_OPER },
    { "CLEARAGAIN", SDL_SCANCODE_CLEARAGAIN },
    { "CRSEL", SDL_SCANCODE_CRSEL },
    { "EXSEL", SDL_SCANCODE_EXSEL },
    { "KP_00", SDL_SCANCODE_KP_00 },
    { "KP_000", SDL_SCANCODE_KP_000 },
    { "THOUSANDSSEPARATOR", SDL_SCANCODE_THOUSANDSSEPARATOR },
    { "DECIMALSEPARATOR", SDL_SCANCODE_DECIMALSEPARATOR },
    { "CURRENCYUNIT", SDL_SCANCODE_CURRENCYUNIT },
    { "CURRENCYSUBUNIT", SDL_SCANCODE_CURRENCYSUBUNIT },
    { "KP_LEFTPAREN", SDL_SCANCODE_KP_LEFTPAREN },
    { "KP_RIGHTPAREN", SDL_SCANCODE_KP_RIGHTPAREN },
    { "KP_LEFTBRACE", SDL_SCANCODE_KP_LEFTBRACE },
    { "KP_RIGHTBRACE", SDL_SCANCODE_KP_RIGHTBRACE },
    { "KP_TAB", SDL_SCANCODE_KP_TAB },
    { "KP_BACKSPACE", SDL_SCANCODE_KP_BACKSPACE },
    { "KP_A", SDL_SCANCODE_KP_A },
    { "KP_B", SDL_SCANCODE_KP_B },
    { "KP_C", SDL_SCANCODE_KP_C },
    { "KP_D", SDL_SCANCODE_KP_D },
    { "KP_E", SDL_SCANCODE_KP_E },
    { "KP_F", SDL_SCANCODE_KP_F },
    { "KP_XOR", SDL_SCANCODE_KP_XOR },
    { "KP_POWER", SDL_SCANCODE_KP_POWER },
    { "KP_PERCENT", SDL_SCANCODE_KP_PERCENT },
    { "KP_LESS", SDL_SCANCODE_KP_LESS },
    { "KP_GREATER", SDL_SCANCODE_KP_GREATER },
    { "KP_AMPERSAND", SDL_SCANCODE_KP_AMPERSAND },
    { "KP_DBLAMPERSAND", SDL_SCANCODE_KP_DBLAMPERSAND },
    { "KP_VERTICALBAR", SDL_SCANCODE_KP_VERTICALBAR },
    { "KP_DBLVERTICALBAR", SDL_SCANCODE_KP_DBLVERTICALBAR },
    { "KP_COLON", SDL_SCANCODE_KP_COLON },
    { "KP_HASH", SDL_SCANCODE_KP_HASH },
    { "KP_SPACE", SDL_SCANCODE_KP_SPACE },
    { "KP_AT", SDL_SCANCODE_KP_AT },
    { "KP_EXCLAM", SDL_SCANCODE_KP_EXCLAM },
    { "KP_MEMSTORE", SDL_SCANCODE_KP_MEMSTORE },
    { "KP_MEMRECALL", SDL_SCANCODE_KP_MEMRECALL },
    { "KP_MEMCLEAR", SDL_SCANCODE_KP_MEMCLEAR },
    { "KP_MEMADD", SDL_SCANCODE_KP_MEMADD },
    { "KP_MEMSUBTRACT", SDL_SCANCODE_KP_MEMSUBTRACT },
    { "KP_MEMMULTIPLY", SDL_SCANCODE_KP_MEMMULTIPLY },
    { "KP_MEMDIVIDE", SDL_SCANCODE_KP_MEMDIVIDE },
    { "KP_PLUSMINUS", SDL_SCANCODE_KP_PLUSMINUS },
    { "KP_CLEAR", SDL_SCANCODE_KP_CLEAR },
    { "KP_CLEARENTRY", SDL_SCANCODE_KP_CLEARENTRY },
    { "KP_BINARY", SDL_SCANCODE_KP_BINARY },
    { "KP_OCTAL", SDL_SCANCODE_KP_OCTAL },
    { "KP_DECIMAL", SDL_SCANCODE_KP_DECIMAL },
    { "KP_HEXADECIMAL", SDL_SCANCODE_KP_HEXADECIMAL },
    { "LCTRL", SDL_SCANCODE_LCTRL },
    { "LSHIFT", SDL_SCANCODE_LSHIFT },
    { "LALT", SDL_SCANCODE_LALT },
    { "LGUI", SDL_SCANCODE_LGUI },
    { "RCTRL", SDL_SCANCODE_RCTRL },
    { "RSHIFT", SDL_SCANCODE_RSHIFT },
    { "RALT", SDL_SCANCODE_RALT },
    { "RGUI", SDL_SCANCODE_RGUI },
    { "MODE", SDL_SCANCODE_MODE },
    { "AUDIONEXT", SDL_SCANCODE_AUDIONEXT },
    { "AUDIOPREV", SDL_SCANCODE_AUDIOPREV },
    { "AUDIOSTOP", SDL_SCANCODE_AUDIOSTOP },
    { "AUDIOPLAY", SDL_SCANCODE_AUDIOPLAY },
    { "AUDIOMUTE", SDL_SCANCODE_AUDIOMUTE },
    { "MEDIASELECT", SDL_SCANCODE_MEDIASELECT },
    { "WWW", SDL_SCANCODE_WWW },
    { "MAIL", SDL_SCANCODE_MAIL },
    { "CALCULATOR", SDL_SCANCODE_CALCULATOR },
    { "COMPUTER", SDL_SCANCODE_COMPUTER },
    { "AC_SEARCH", SDL_SCANCODE_AC_SEARCH },
    { "AC_HOME", SDL_SCANCODE_AC_HOME },
    { "AC_BACK", SDL_SCANCODE_AC_BACK },
    { "AC_FORWARD", SDL_SCANCODE_AC_FORWARD },
    { "AC_STOP", SDL_SCANCODE_AC_STOP },
    { "AC_REFRESH", SDL_SCANCODE_AC_REFRESH },
    { "AC_BOOKMARKS", SDL_SCANCODE_AC_BOOKMARKS },
    { "BRIGHTNESSDOWN", SDL_SCANCODE_BRIGHTNESSDOWN },
    { "BRIGHTNESSUP", SDL_SCANCODE_BRIGHTNESSUP },
    { "DISPLAYSWITCH", SDL_SCANCODE_DISPLAYSWITCH },
    { "KBDILLUMTOGGLE", SDL_SCANCODE_KBDILLUMTOGGLE },
    { "KBDILLUMDOWN", SDL_SCANCODE_KBDILLUMDOWN },
    { "KBDILLUMUP", SDL_SCANCODE_KBDILLUMUP },
    { "EJECT", SDL_SCANCODE_EJECT },
    { "SLEEP", SDL_SCANCODE_SLEEP },
    { "APP1", SDL_SCANCODE_APP1 },
    { "APP2", SDL_SCANCODE_APP2 },
    { "AUDIOREWIND", SDL_SCANCODE_AUDIOREWIND },
    { "AUDIOFASTFORWARD", SDL_SCANCODE_AUDIOFASTFORWARD },
    { "SOFTLEFT", SDL_SCANCODE_SOFTLEFT },
    { "SOFTRIGHT", SDL_SCANCODE_SOFTRIGHT },
    { "CALL", SDL_SCANCODE_CALL },
    { "ENDCALL", SDL_SCANCODE_ENDCALL },
};

CONFIG_DEFINE_ENUM_TEMPLATE(EChannelConfiguration)
{
    { "Stereo",   EChannelConfiguration::Stereo },
    { "Surround", EChannelConfiguration::Surround }
};

CONFIG_DEFINE_ENUM_TEMPLATE(EVoiceLanguage)
{
    { "English",  EVoiceLanguage::English },
    { "Japanese", EVoiceLanguage::Japanese }
};

CONFIG_DEFINE_ENUM_TEMPLATE(EGraphicsAPI)
{
#ifdef UNLEASHED_RECOMP_D3D12
    { "D3D12",  EGraphicsAPI::D3D12 },
#endif
    { "Vulkan", EGraphicsAPI::Vulkan }
};

CONFIG_DEFINE_ENUM_TEMPLATE(EWindowState)
{
    { "Normal",    EWindowState::Normal },
    { "Maximised", EWindowState::Maximised },
    { "Maximized", EWindowState::Maximised }
};

CONFIG_DEFINE_ENUM_TEMPLATE(EAspectRatio)
{
    { "Auto", EAspectRatio::Auto },
    { "16:9", EAspectRatio::Wide },
    { "4:3",  EAspectRatio::Narrow },
    { "Original 4:3",  EAspectRatio::OriginalNarrow },
};

CONFIG_DEFINE_ENUM_TEMPLATE(ETripleBuffering)
{
    { "Auto", ETripleBuffering::Auto },
    { "On",   ETripleBuffering::On },
    { "Off",  ETripleBuffering::Off }
};

CONFIG_DEFINE_ENUM_TEMPLATE(EAntiAliasing)
{
    { "None",    EAntiAliasing::None },
    { "2x MSAA", EAntiAliasing::MSAA2x },
    { "4x MSAA", EAntiAliasing::MSAA4x },
    { "8x MSAA", EAntiAliasing::MSAA8x }
};

CONFIG_DEFINE_ENUM_TEMPLATE(EShadowResolution)
{
    { "Original", EShadowResolution::Original },
    { "512",      EShadowResolution::x512 },
    { "1024",     EShadowResolution::x1024 },
    { "2048",     EShadowResolution::x2048 },
    { "4096",     EShadowResolution::x4096 },
    { "8192",     EShadowResolution::x8192 },
};

CONFIG_DEFINE_ENUM_TEMPLATE(EGITextureFiltering)
{
    { "Bilinear", EGITextureFiltering::Bilinear },
    { "Bicubic",  EGITextureFiltering::Bicubic }
};

CONFIG_DEFINE_ENUM_TEMPLATE(EDepthOfFieldQuality)
{
    { "Auto",   EDepthOfFieldQuality::Auto },
    { "Low",    EDepthOfFieldQuality::Low },
    { "Medium", EDepthOfFieldQuality::Medium },
    { "High",   EDepthOfFieldQuality::High },
    { "Ultra",  EDepthOfFieldQuality::Ultra }
};

CONFIG_DEFINE_ENUM_TEMPLATE(EMotionBlur)
{
    { "Off",      EMotionBlur::Off },
    { "Original", EMotionBlur::Original },
    { "Enhanced", EMotionBlur::Enhanced }
};

CONFIG_DEFINE_ENUM_TEMPLATE(ECutsceneAspectRatio)
{
    { "Original", ECutsceneAspectRatio::Original },
    { "Unlocked", ECutsceneAspectRatio::Unlocked }
};

CONFIG_DEFINE_ENUM_TEMPLATE(EUIScaleMode)
{
    { "Edge",    EUIScaleMode::Edge },
    { "Centre",  EUIScaleMode::Centre },
    { "Center",  EUIScaleMode::Centre }
};

#undef  CONFIG_DEFINE
#define CONFIG_DEFINE(section, type, name, defaultValue) \
    ConfigDef<type> Config::name{section, #name, defaultValue};

#undef  CONFIG_DEFINE_HIDDEN
#define CONFIG_DEFINE_HIDDEN(section, type, name, defaultValue) \
    ConfigDef<type, true> Config::name{section, #name, defaultValue};

#undef  CONFIG_DEFINE_LOCALISED
#define CONFIG_DEFINE_LOCALISED(section, type, name, defaultValue) \
    extern CONFIG_LOCALE g_##name##_locale; \
    ConfigDef<type> Config::name{section, #name, &g_##name##_locale, defaultValue};

#undef  CONFIG_DEFINE_ENUM
#define CONFIG_DEFINE_ENUM(section, type, name, defaultValue) \
    ConfigDef<type> Config::name{section, #name, defaultValue, &g_##type##_template};

#undef  CONFIG_DEFINE_ENUM_LOCALISED
#define CONFIG_DEFINE_ENUM_LOCALISED(section, type, name, defaultValue) \
    extern CONFIG_LOCALE g_##name##_locale; \
    extern CONFIG_ENUM_LOCALE(type) g_##type##_locale; \
    ConfigDef<type> Config::name{section, #name, &g_##name##_locale, defaultValue, &g_##type##_template, &g_##type##_locale};

#undef  CONFIG_DEFINE_CALLBACK
#define CONFIG_DEFINE_CALLBACK(section, type, name, defaultValue, readCallback) \
    extern CONFIG_LOCALE g_##name##_locale; \
    ConfigDef<type> Config::name{section, #name, defaultValue, [](ConfigDef<type>* def) readCallback};

#include "config_def.h"

// CONFIG_DEFINE
template<typename T, bool isHidden>
ConfigDef<T, isHidden>::ConfigDef(std::string section, std::string name, T defaultValue) : Section(section), Name(name), DefaultValue(defaultValue)
{
    g_configDefinitions.emplace_back(this);
}

// CONFIG_DEFINE_LOCALISED
template<typename T, bool isHidden>
ConfigDef<T, isHidden>::ConfigDef(std::string section, std::string name, CONFIG_LOCALE* nameLocale, T defaultValue) : Section(section), Name(name), Locale(nameLocale), DefaultValue(defaultValue)
{
    g_configDefinitions.emplace_back(this);
}

// CONFIG_DEFINE_ENUM
template<typename T, bool isHidden>
ConfigDef<T, isHidden>::ConfigDef(std::string section, std::string name, T defaultValue, std::unordered_map<std::string, T>* enumTemplate) : Section(section), Name(name), DefaultValue(defaultValue), EnumTemplate(enumTemplate)
{
    for (const auto& pair : *EnumTemplate)
        EnumTemplateReverse[pair.second] = pair.first;

    g_configDefinitions.emplace_back(this);
}

// CONFIG_DEFINE_ENUM_LOCALISED
template<typename T, bool isHidden>
ConfigDef<T, isHidden>::ConfigDef(std::string section, std::string name, CONFIG_LOCALE* nameLocale, T defaultValue, std::unordered_map<std::string, T>* enumTemplate, CONFIG_ENUM_LOCALE(T)* enumLocale) : Section(section), Name(name), Locale(nameLocale), DefaultValue(defaultValue), EnumTemplate(enumTemplate), EnumLocale(enumLocale)
{
    for (const auto& pair : *EnumTemplate)
        EnumTemplateReverse[pair.second] = pair.first;

    g_configDefinitions.emplace_back(this);
}

// CONFIG_DEFINE_CALLBACK
template<typename T, bool isHidden>
ConfigDef<T, isHidden>::ConfigDef(std::string section, std::string name, T defaultValue, std::function<void(ConfigDef<T, isHidden>*)> callback) : Section(section), Name(name), DefaultValue(defaultValue), Callback(callback)
{
    g_configDefinitions.emplace_back(this);
}

template<typename T, bool isHidden>
ConfigDef<T, isHidden>::~ConfigDef() = default;

template<typename T, bool isHidden>
bool ConfigDef<T, isHidden>::IsHidden()
{
    return isHidden && !IsLoadedFromConfig;
}

template<typename T, bool isHidden>
void ConfigDef<T, isHidden>::ReadValue(toml::v3::ex::parse_result& toml)
{
    if (auto pSection = toml[Section].as_table())
    {
        const auto& section = *pSection;

        if constexpr (std::is_same<T, std::string>::value)
        {
            Value = section[Name].value_or<std::string>(DefaultValue);
        }
        else if constexpr (std::is_enum_v<T>)
        {
            auto value = section[Name].value_or(std::string());
            auto it = EnumTemplate->find(value);

            if (it != EnumTemplate->end())
            {
                Value = it->second;
            }
            else
            {
                Value = DefaultValue;
            }
        }
        else
        {
            Value = section[Name].value_or(DefaultValue);
        }

        if (Callback)
            Callback(this);

        if (pSection->contains(Name))
            IsLoadedFromConfig = true;
    }
}

template<typename T, bool isHidden>
void ConfigDef<T, isHidden>::MakeDefault()
{
    Value = DefaultValue;
}

template<typename T, bool isHidden>
std::string_view ConfigDef<T, isHidden>::GetSection() const
{
    return Section;
}

template<typename T, bool isHidden>
std::string_view ConfigDef<T, isHidden>::GetName() const
{
    return Name;
}

template<typename T, bool isHidden>
std::string ConfigDef<T, isHidden>::GetNameLocalised(ELanguage language) const
{
    if (!Locale)
        return Name;

    if (!Locale->count(language))
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

    return std::get<0>(Locale->at(language));
}

template<typename T, bool isHidden>
std::string ConfigDef<T, isHidden>::GetDescription(ELanguage language) const
{
    if (!Locale)
        return "";

    if (!Locale->count(language))
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

    return std::get<1>(Locale->at(language));
}

template<typename T, bool isHidden>
bool ConfigDef<T, isHidden>::IsDefaultValue() const
{
    return Value == DefaultValue;
}

template<typename T, bool isHidden>
const void* ConfigDef<T, isHidden>::GetValue() const
{
    return &Value;
}

template<typename T, bool isHidden>
std::string ConfigDef<T, isHidden>::GetValueLocalised(ELanguage language) const
{
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

template<typename T, bool isHidden>
std::string ConfigDef<T, isHidden>::GetValueDescription(ELanguage language) const
{
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

template<typename T, bool isHidden>
std::string ConfigDef<T, isHidden>::GetDefinition(bool withSection) const
{
    std::string result;

    if (withSection)
        result += "[" + Section + "]\n";

    result += Name + " = " + ToString();

    return result;
}

template<typename T, bool isHidden>
std::string ConfigDef<T, isHidden>::ToString(bool strWithQuotes) const
{
    std::string result = "N/A";

    if constexpr (std::is_same_v<T, std::string>)
    {
        result = fmt::format("{}", Value);

        if (strWithQuotes)
            result = fmt::format("\"{}\"", result);
    }
    else if constexpr (std::is_enum_v<T>)
    {
        auto it = EnumTemplateReverse.find(Value);

        if (it != EnumTemplateReverse.end())
            result = fmt::format("{}", it->second);

        if (strWithQuotes)
            result = fmt::format("\"{}\"", result);
    }
    else
    {
        result = fmt::format("{}", Value);
    }

    return result;
}

template<typename T, bool isHidden>
void ConfigDef<T, isHidden>::GetLocaleStrings(std::vector<std::string_view>& localeStrings) const
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

std::filesystem::path Config::GetConfigPath()
{
    return GetUserPath() / "config.toml";
}

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
            toml = toml::parse(tomlStream);

        for (auto def : g_configDefinitions)
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

    for (auto def : g_configDefinitions)
    {
        if (def->IsHidden())
            continue;

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
