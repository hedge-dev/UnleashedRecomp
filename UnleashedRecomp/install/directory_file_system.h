#pragma once

#include <filesystem>

#include "virtual_file_system.h"

struct DirectoryFileSystem : VirtualFileSystem
{
    std::filesystem::path directoryPath;
    std::string name;

    DirectoryFileSystem(const std::filesystem::path &directoryPath)
    {
        this->directoryPath = directoryPath;
        name = (const char *)(directoryPath.filename().u8string().data());
    }

    bool load(const std::string &path, uint8_t *fileData, size_t fileDataMaxByteCount) const override
    {
        std::ifstream fileStream(directoryPath / std::filesystem::path(std::u8string_view((const char8_t *)(path.c_str()))), std::ios::binary);
        if (fileStream.is_open())
        {
            fileStream.read((char *)(fileData), fileDataMaxByteCount);
            return !fileStream.bad();
        }
        else
        {
            return false;
        }
    }

    bool read(const std::string &path, size_t offset, uint8_t *fileData, size_t size) const override
    {
        std::ifstream fileStream(directoryPath / std::filesystem::path(std::u8string_view((const char8_t *)(path.c_str()))), std::ios::binary);
        if (!fileStream.is_open())
        {
            return false;
        }

        fileStream.seekg(offset, std::ios::beg);
        fileStream.read(reinterpret_cast<char *>(fileData), size);
        return !fileStream.fail() && static_cast<size_t>(fileStream.gcount()) == size;
    }

    size_t getSize(const std::string &path) const override
    {
        std::error_code ec;
        size_t fileSize = std::filesystem::file_size(directoryPath / std::filesystem::path(std::u8string_view((const char8_t *)(path.c_str()))), ec);
        if (!ec)
        {
            return fileSize;
        }
        else
        {
            return 0;
        }
    }

    bool exists(const std::string &path) const override
    {
        if (path.empty())
        {
            return false;
        }

        return std::filesystem::exists(directoryPath / std::filesystem::path(std::u8string_view((const char8_t *)(path.c_str()))));
    }

    const std::string &getName() const override
    {
        return name;
    }

    static std::unique_ptr<VirtualFileSystem> create(const std::filesystem::path &directoryPath)
    {
        if (std::filesystem::exists(directoryPath))
        {
            return std::make_unique<DirectoryFileSystem>(directoryPath);
        }
        else
        {
            return nullptr;
        }
    }
};
