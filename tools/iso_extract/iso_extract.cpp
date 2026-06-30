#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include <iso_file_system.h>

static bool ExtractFile(const ISOFileSystem& iso, const std::string& sourcePath, const std::filesystem::path& outputPath)
{
    std::vector<uint8_t> data;
    size_t dataSize = iso.getSize(sourcePath);
    if (dataSize == 0)
    {
        std::cerr << "File not found in ISO: " << sourcePath << "\n";
        return false;
    }

    data.resize(dataSize);
    if (!iso.load(sourcePath, data.data(), data.size()))
    {
        std::cerr << "File not found in ISO: " << sourcePath << "\n";
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
        std::cerr << "Usage: iso_extract <game.iso> <iso-path> <output-path> [<iso-path> <output-path> ...]\n";
        return 2;
    }

    auto iso = ISOFileSystem::create(argv[1]);
    if (!iso)
    {
        std::cerr << "Failed to open Xbox 360 ISO: " << argv[1] << "\n";
        return 1;
    }

    bool ok = true;
    for (int i = 2; i < argc; i += 2)
    {
        ok = ExtractFile(*iso, argv[i], argv[i + 1]) && ok;
    }

    return ok ? 0 : 1;
}
