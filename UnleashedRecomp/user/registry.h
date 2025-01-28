#pragma once

class Registry
{
public:
    inline static std::string RootDirectoryPath;

    static void Load();
    static void Save();
};
