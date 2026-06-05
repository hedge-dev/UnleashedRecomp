#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include <xcontent_file_system.h>

static bool ExtractFile(const XContentFileSystem& content, const std::string& sourcePath, const std::filesystem::path& outputPath)
{
    size_t dataSize = content.getSize(sourcePath);
    if (dataSize == 0)
    {
        std::cerr << "File not found in XContent package: " << sourcePath << "\n";
        return false;
    }

    std::vector<uint8_t> data(dataSize);
    if (!content.load(sourcePath, data.data(), data.size()))
    {
        std::cerr << "Failed to read XContent file: " << sourcePath << "\n";
        return false;
    }

    std::error_code ec;
    std::filesystem::create_directories(outputPath.parent_path(), ec);
    if (ec)
    {
        std::cerr << "Failed to create output directory: " << outputPath.parent_path() << "\n";
        return false;
    }

    std::ofstream output(outputPath, std::ios::binary);
    if (!output)
    {
        std::cerr << "Failed to open output file: " << outputPath << "\n";
        return false;
    }

    output.write(reinterpret_cast<const char*>(data.data()), static_cast<std::streamsize>(data.size()));
    if (!output)
    {
        std::cerr << "Failed to write output file: " << outputPath << "\n";
        return false;
    }

    return true;
}

int main(int argc, char** argv)
{
    if (argc < 4 || ((argc - 2) % 2) != 0)
    {
        std::cerr << "Usage: xcontent_extract <package> <package-path> <output-path> [<package-path> <output-path> ...]\n";
        return 2;
    }

    auto content = XContentFileSystem::create(argv[1]);
    if (!content)
    {
        std::cerr << "Failed to open XContent package: " << argv[1] << "\n";
        return 1;
    }

    bool ok = true;
    for (int i = 2; i < argc; i += 2)
    {
        ok = ExtractFile(*content, argv[i], argv[i + 1]) && ok;
    }

    return ok ? 0 : 1;
}
