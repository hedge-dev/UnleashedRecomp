#pragma once

#define USER_DIRECTORY "SWA"

#define TOML_FILE "config.toml"

#define CONFIG_DEFINE(section, type, name, defaultValue) \
    inline static ConfigDef<type> name{section, #name, defaultValue};

#define CONFIG_DEFINE_HIDE(section, type, name, defaultValue) \
    inline static ConfigDef<type, false> name{section, #name, defaultValue};

#define CONFIG_DEFINE_ENUM_TEMPLATE(type) \
    inline static std::unordered_map<std::string, type> g_##type##_template =

#define CONFIG_DEFINE_ENUM(section, type, name, defaultValue) \
    inline static ConfigDef<type> name{section, #name, defaultValue, g_##type##_template};

#define CONFIG_DEFINE_ENUM_HIDE(section, type, name, defaultValue) \
    inline static ConfigDef<type, false> name{section, #name, defaultValue, g_##type##_template};

#define CONFIG_DEFINE_IMPL(section, type, name, defaultValue, readImpl) \
    inline static ConfigDef<type> name{section, #name, defaultValue, [](ConfigDef<type, true>* def, const toml::v3::table& table) readImpl};

#define CONFIG_DEFINE_CALLBACK(section, type, name, defaultValue, readCallback) \
    inline static ConfigDef<type> name{section, #name, defaultValue, [](ConfigDef<type, true>* def) readCallback};

#define CONFIG_GET_DEFAULT(name) Config::name.DefaultValue
#define CONFIG_SET_DEFAULT(name) Config::name.MakeDefault();

#define WINDOWPOS_CENTRED 0x2FFF0000

class IConfigDef
{
public:
    virtual ~IConfigDef() = default;
    virtual void ReadValue(toml::v3::ex::parse_result& toml) = 0;
    virtual void MakeDefault() = 0;
    virtual bool IsMenuOption() const = 0;
    virtual std::string_view GetSection() const = 0;
    virtual std::string_view GetName() const = 0;
    virtual void* GetValue() = 0;
    virtual std::string GetDefinition(bool withSection = false) const = 0;
    virtual std::string ToString() const = 0;
};

template<typename T, bool isMenuOption = true>
class ConfigDef : public IConfigDef
{
public:
    std::string Section{};
    std::string Name{};
    T DefaultValue{};
    T Value{ DefaultValue };
    std::unordered_map<std::string, T> EnumTemplate{};
    std::unordered_map<T, std::string> EnumTemplateReverse{};
    std::function<void(ConfigDef<T, isMenuOption>*, const toml::v3::table&)> ReadImpl;
    std::function<void(ConfigDef<T, isMenuOption>*)> ReadCallback;

    ConfigDef(std::string section, std::string name, T defaultValue)
        : Section(section), Name(name), DefaultValue(defaultValue)
    {
        Config::Definitions.emplace_back(this);
    }

    ConfigDef(std::string section, std::string name, T defaultValue, std::unordered_map<std::string, T> enumTemplate)
        : Section(section), Name(name), DefaultValue(defaultValue), EnumTemplate(enumTemplate)
    {
        for (const auto& pair : EnumTemplate)
            EnumTemplateReverse[pair.second] = pair.first;

        Config::Definitions.emplace_back(this);
    }

    ConfigDef(std::string section, std::string name, T defaultValue, std::function<void(ConfigDef<T, isMenuOption>*)> readCallback)
        : Section(section), Name(name), DefaultValue(defaultValue), ReadCallback(readCallback)
    {
        Config::Definitions.emplace_back(this);
    }

    ConfigDef(std::string section, std::string name, T defaultValue, std::function<void(ConfigDef<T, isMenuOption>*, const toml::v3::table&)> readImpl)
        : Section(section), Name(name), DefaultValue(defaultValue), ReadImpl(readImpl)
    {
        Config::Definitions.emplace_back(this);
    }

    void ReadValue(toml::v3::ex::parse_result& toml) override
    {
        if (auto pSection = toml[Section].as_table())
        {
            const auto& section = *pSection;

            if (ReadImpl)
            {
                ReadImpl(this, section);
            }
            else
            {
                if constexpr (std::is_same<T, std::string>::value)
                {
                    Value = section[Name].value_or<std::string>(DefaultValue);
                }
                else if constexpr (std::is_enum_v<T>)
                {
                    auto it = EnumTemplate.begin();

                    Value = EnumTemplate[section[Name].value_or<std::string>(static_cast<std::string>(it->first))];
                }
                else
                {
                    Value = section[Name].value_or(DefaultValue);
                }

                if (ReadCallback)
                    ReadCallback(this);
            }
        }
    }

    void MakeDefault() override
    {
        Value = DefaultValue;
    }

    bool IsMenuOption() const override
    {
        return isMenuOption;
    }

    std::string_view GetSection() const override
    {
        return Section;
    }

    std::string_view GetName() const override
    {
        return Name;
    }

    void* GetValue() override
    {
        return &Value;
    }

    std::string GetDefinition(bool withSection = false) const override
    {
        std::string result;

        if (withSection)
            result += "[" + Section + "]\n";

        result += Name + " = " + ToString();

        return result;
    }

    std::string ToString() const override
    {
        std::string result = "\"N/A\"";

        if constexpr (std::is_same_v<T, std::string>)
        {
            result = std::format("\"{}\"", Value);
        }
        else if constexpr (std::is_enum_v<T>)
        {
            auto it = EnumTemplateReverse.find(Value);

            if (it != EnumTemplateReverse.end())
                result = std::format("\"{}\"", it->second);
        }
        else
        {
            result = std::format("{}", Value);
        }

        return result;
    }

    ConfigDef& operator=(const ConfigDef& other)
    {
        if (this != &other)
            Value = other.Value;

        return *this;
    }

    operator T() const
    {
        return Value;
    }

    void operator=(const T& other)
    {
        Value = other;
    }
};

enum class ELanguage : uint32_t
{
    English = 1,
    Japanese,
    German,
    French,
    Spanish,
    Italian
};

CONFIG_DEFINE_ENUM_TEMPLATE(ELanguage)
{
    { "English",  ELanguage::English  },
    { "Japanese", ELanguage::Japanese },
    { "German",   ELanguage::German   },
    { "French",   ELanguage::French   },
    { "Spanish",  ELanguage::Spanish  },
    { "Italian",  ELanguage::Italian  }
};

enum class EScoreBehaviour : uint32_t
{
    CheckpointReset,
    CheckpointRetain
};

CONFIG_DEFINE_ENUM_TEMPLATE(EScoreBehaviour)
{
    { "CheckpointReset",  EScoreBehaviour::CheckpointReset  },
    { "CheckpointRetain", EScoreBehaviour::CheckpointRetain }
};

enum class EVoiceLanguage : uint32_t
{
    English,
    Japanese
};

CONFIG_DEFINE_ENUM_TEMPLATE(EVoiceLanguage)
{
    { "English",  EVoiceLanguage::English  },
    { "Japanese", EVoiceLanguage::Japanese }
};

enum class EGraphicsAPI : uint32_t
{
    D3D12,
    Vulkan
};

CONFIG_DEFINE_ENUM_TEMPLATE(EGraphicsAPI)
{
    { "D3D12",  EGraphicsAPI::D3D12  },
    { "Vulkan", EGraphicsAPI::Vulkan }
};

enum class EWindowState : uint32_t
{
    Normal,
    Maximised
};

CONFIG_DEFINE_ENUM_TEMPLATE(EWindowState)
{
    { "Normal",    EWindowState::Normal    },
    { "Maximised", EWindowState::Maximised },
    { "Maximized", EWindowState::Maximised }
};

enum class EShadowResolution : int32_t
{
    Original = -1,
    x512     = 512,
    x1024    = 1024,
    x2048    = 2048,
    x4096    = 4096,
    x8192    = 8192
};

CONFIG_DEFINE_ENUM_TEMPLATE(EShadowResolution)
{
    { "Original", EShadowResolution::Original },
    { "512",      EShadowResolution::x512     },
    { "1024",     EShadowResolution::x1024    },
    { "2048",     EShadowResolution::x2048    },
    { "4096",     EShadowResolution::x4096    },
    { "8192",     EShadowResolution::x8192    },
};

enum class EGITextureFiltering : uint32_t
{
    Linear,
    Bicubic
};

CONFIG_DEFINE_ENUM_TEMPLATE(EGITextureFiltering)
{
    { "Linear",  EGITextureFiltering::Linear  },
    { "Bicubic", EGITextureFiltering::Bicubic }
};

enum class EMovieScaleMode : uint32_t
{
    Stretch,
    Fit,
    Fill
};

CONFIG_DEFINE_ENUM_TEMPLATE(EMovieScaleMode)
{
    { "Stretch", EMovieScaleMode::Stretch },
    { "Fit",     EMovieScaleMode::Fit     },
    { "Fill",    EMovieScaleMode::Fill    }
};

enum class EUIScaleMode : uint32_t
{
    Stretch,
    Edge,
    Centre
};

CONFIG_DEFINE_ENUM_TEMPLATE(EUIScaleMode)
{
    { "Stretch", EUIScaleMode::Stretch },
    { "Edge",    EUIScaleMode::Edge    },
    { "Centre",  EUIScaleMode::Centre  },
    { "Center",  EUIScaleMode::Centre  }
};
