#pragma once

struct ModLoader
{
    static std::filesystem::path RedirectPath(std::string_view path);

    static std::vector<std::filesystem::path>* GetIncludeDirectories(size_t modIndex);

    static void Init();
};
