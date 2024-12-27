inline size_t IniFile::hashStr(const std::string_view& str)
{
    return XXH3_64bits(str.data(), str.size());
}

inline bool IniFile::isWhitespace(char value)
{
    return value == ' ' || value == '\t';
}

inline bool IniFile::isNewLine(char value)
{
    return value == '\n' || value == '\r';
}

inline bool IniFile::read(const std::filesystem::path& filePath)
{
    std::ifstream file(filePath, std::ios::binary);
    if (!file.good())
        return false;

    file.seekg(0, std::ios::end);

    const size_t dataSize = static_cast<size_t>(file.tellg());
    const auto data = std::make_unique<char[]>(dataSize + 1);
    data[dataSize] = '\0';

    file.seekg(0, std::ios::beg);
    file.read(data.get(), dataSize);

    file.close();

    Section* section = nullptr;
    const char* dataPtr = data.get();

    while (dataPtr < data.get() + dataSize)
    {
        if (*dataPtr == ';')
        {
            while (*dataPtr != '\0' && !isNewLine(*dataPtr))
                ++dataPtr;
        }
        else if (*dataPtr == '[')
        {
            ++dataPtr;
            const char* endPtr = dataPtr;
            while (*endPtr != '\0' && !isNewLine(*endPtr) && *endPtr != ']')
                ++endPtr;

            if (*endPtr != ']')
                return false;

            std::string sectionName(dataPtr, endPtr - dataPtr);
            section = &m_sections[hashStr(sectionName)];
            section->name = std::move(sectionName);

            dataPtr = endPtr + 1;
        }
        else if (!isWhitespace(*dataPtr) && !isNewLine(*dataPtr))
        {
            if (section == nullptr)
                return false;

            const char* endPtr;
            if (*dataPtr == '"')
            {
                ++dataPtr;
                endPtr = dataPtr;

                while (*endPtr != '\0' && !isNewLine(*endPtr) && *endPtr != '"')
                    ++endPtr;

                if (*endPtr != '"')
                    return false;
            }
            else
            {
                endPtr = dataPtr;

                while (*endPtr != '\0' && !isNewLine(*endPtr) && !isWhitespace(*endPtr) && *endPtr != '=')
                    ++endPtr;

                if (!isNewLine(*endPtr) && !isWhitespace(*endPtr) && *endPtr != '=')
                    return false;
            }

            std::string propertyName(dataPtr, endPtr - dataPtr);
            auto& property = section->properties[hashStr(propertyName)];
            property.name = std::move(propertyName);

            dataPtr = endPtr;
            while (*dataPtr != '\0' && !isNewLine(*dataPtr) && *dataPtr != '=')
                ++dataPtr;

            if (*dataPtr == '=')
            {
                ++dataPtr;

                while (*dataPtr != '\0' && isWhitespace(*dataPtr))
                    ++dataPtr;

                if (*dataPtr == '"')
                {
                    ++dataPtr;
                    endPtr = dataPtr;

                    while (*endPtr != '\0' && !isNewLine(*endPtr) && *endPtr != '"')
                        ++endPtr;

                    if (*endPtr != '"')
                        return false;
                }
                else
                {
                    endPtr = dataPtr;

                    while (*endPtr != '\0' && !isNewLine(*endPtr) && !isWhitespace(*endPtr))
                        ++endPtr;
                }

                property.value = std::string(dataPtr, endPtr - dataPtr);
                dataPtr = endPtr + 1;
            }
        }
        else
        {
            ++dataPtr;
        }
    }

    return true;
}

inline std::string IniFile::getString(const std::string_view& sectionName, const std::string_view& propertyName, std::string defaultValue) const
{
    const auto sectionPair = m_sections.find(hashStr(sectionName));
    if (sectionPair != m_sections.end())
    {
        const auto propertyPair = sectionPair->second.properties.find(hashStr(propertyName));
        if (propertyPair != sectionPair->second.properties.end())
            return propertyPair->second.value;
    }
    return defaultValue;
}

inline bool IniFile::getBool(const std::string_view& sectionName, const std::string_view& propertyName, bool defaultValue) const
{
    const auto sectionPair = m_sections.find(hashStr(sectionName));
    if (sectionPair != m_sections.end())
    {
        const auto propertyPair = sectionPair->second.properties.find(hashStr(propertyName));
        if (propertyPair != sectionPair->second.properties.end() && !propertyPair->second.value.empty())
        {
            const char firstChar = propertyPair->second.value[0];
            return firstChar == 't' || firstChar == 'T' || firstChar == 'y' || firstChar == 'Y' || firstChar == '1';
        }
    }
    return defaultValue;
}

inline bool IniFile::contains(const std::string_view& sectionName) const
{
    return m_sections.contains(hashStr(sectionName));
}

template <typename T>
T IniFile::get(const std::string_view& sectionName, const std::string_view& propertyName, T defaultValue) const
{
    const auto sectionPair = m_sections.find(hashStr(sectionName));
    if (sectionPair != m_sections.end())
    {
        const auto propertyPair = sectionPair->second.properties.find(hashStr(propertyName));
        if (propertyPair != sectionPair->second.properties.end())
        {
            T value{};
            const auto result = std::from_chars(propertyPair->second.value.data(), 
                propertyPair->second.value.data() + propertyPair->second.value.size(), value);

            if (result.ec == std::errc{})
                return value;
        }
    }
    return defaultValue;
}

template<typename T>
inline void IniFile::enumerate(const T& function) const
{
    for (const auto& [_, section] : m_sections)
    {
        for (auto& property : section.properties)
            function(section.name, property.second.name, property.second.value);
    }
}

template <typename T>
void IniFile::enumerate(const std::string_view& sectionName, const T& function) const
{
    const auto sectionPair = m_sections.find(hashStr(sectionName));
    if (sectionPair != m_sections.end())
    {
        for (const auto& property : sectionPair->second.properties)
            function(property.second.name, property.second.value);
    }
}
