#pragma once

struct ModLoader
{
    static inline bool s_isLogTypeConsole;

    static inline std::filesystem::path s_saveFilePath;
    
    static std::filesystem::path ResolvePath(std::string_view path);

    static std::vector<std::filesystem::path>* GetIncludeDirectories(size_t modIndex);

    static void Init();
};
