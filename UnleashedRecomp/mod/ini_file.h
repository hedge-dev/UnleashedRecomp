#pragma once

#include <xxHashMap.h>

class IniFile
{
protected:
    struct Property
    {
        std::string name;
        std::string value;
    };

    struct Section
    {
        std::string name;
        xxHashMap<Property> properties;
    };

    xxHashMap<Section> m_sections;

    static size_t hashStr(const std::string_view& str);

    static bool isWhitespace(char value);
    static bool isNewLine(char value);

public:
    bool read(const std::filesystem::path& filePath);

    std::string getString(const std::string_view& sectionName, const std::string_view& propertyName, std::string defaultValue) const;

    bool getBool(const std::string_view& sectionName, const std::string_view& propertyName, bool defaultValue) const;

    template<typename T>
    T get(const std::string_view& sectionName, const std::string_view& propertyName, T defaultValue) const;

    template<typename T>
    void enumerate(const T& function) const;

    template<typename T>
    void enumerate(const std::string_view& sectionName, const T& function) const;

    bool contains(const std::string_view& sectionName) const;
};

#include "ini_file.inl"
