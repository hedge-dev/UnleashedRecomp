#pragma once

#define USER_DIRECTORY "SWA"

#define TOML_FILE "config.toml"

#define CONFIG_DEFINE(section, type, name, defaultValue) \
    inline static ConfigDef<type> name{section, #name, defaultValue};

#define CONFIG_DEFINE_ENUM_TEMPLATE(type) \
    inline static std::unordered_map<std::string, type> g_##type##_template =

#define CONFIG_DEFINE_ENUM(section, type, name, defaultValue) \
    inline static ConfigDef<type> name{section, #name, defaultValue, g_##type##_template};

#define CONFIG_DEFINE_CALLBACK(section, type, name, defaultValue, readCallback) \
    inline static ConfigDef<type> name{section, #name, defaultValue, [](ConfigDef<type>* def) readCallback};

#define CONFIG_GET_DEFAULT(name) Config::name.DefaultValue
#define CONFIG_SET_DEFAULT(name) Config::name.MakeDefault();

#define WINDOWPOS_CENTRED 0x2FFF0000

class IConfigDef
{
public:
    virtual ~IConfigDef() = default;
    virtual void ReadValue(toml::v3::ex::parse_result& toml) = 0;
    virtual void MakeDefault() = 0;
    virtual std::string GetSection() const = 0;
    virtual std::string GetName() const = 0;
    virtual std::string GetDefinition(bool withSection = false) const = 0;
    virtual std::string ToString() const = 0;
};

template<typename T>
class ConfigDef : public IConfigDef
{
public:
    std::string Section{};
    std::string Name{};
    T DefaultValue{};
    T Value{ DefaultValue };
    std::unordered_map<std::string, T> EnumTemplate{};
    std::unordered_map<T, std::string> EnumTemplateReverse{};
    std::function<void(ConfigDef<T>*)> ReadCallback;

    // CONFIG_DEFINE
    ConfigDef(std::string section, std::string name, T defaultValue);

    // CONFIG_DEFINE_ENUM
    ConfigDef(std::string section, std::string name, T defaultValue, std::unordered_map<std::string, T> enumTemplate);

    // CONFIG_DEFINE_CALLBACK
    ConfigDef(std::string section, std::string name, T defaultValue, std::function<void(ConfigDef<T>*)> readCallback);

    void ReadValue(toml::v3::ex::parse_result& toml) override
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

    void MakeDefault() override
    {
        Value = DefaultValue;
    }

    std::string GetSection() const override
    {
        return Section;
    }

    std::string GetName() const override
    {
        return Name;
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
        if constexpr (std::is_same<T, std::string>::value)
        {
            return std::format("\"{}\"", Value);
        }
        else if constexpr (std::is_enum_v<T>)
        {
            auto it = EnumTemplateReverse.find(Value);

            if (it != EnumTemplateReverse.end())
            {
                return std::format("\"{}\"", it->second);
            }
            else
            {
                return "\"N/A\"";
            }
        }
        else
        {
            return std::format("{}", Value);
        }
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
