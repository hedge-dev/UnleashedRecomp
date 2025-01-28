#pragma once

class Registry
{
public:
    inline static std::filesystem::path RootDirectoryPath;

    static void Load();
    static void Save();
};
