#include "bc_diff.h"
#include <cassert>
#include <filesystem>
#include <vector>
#include <xxhash.h>

static std::vector<uint8_t> readAllBytes(const char* filePath)
{
    FILE* file = fopen(filePath, "rb");

    if (!file)
        return {};

    fseek(file, 0, SEEK_END);

    long fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    std::vector<uint8_t> data(fileSize);
    fread(data.data(), 1, fileSize, file);

    fclose(file);

    return data;
}

int main(int argc, char** argv)
{
    if (argc != 4)
    {
        printf("Usage: %s [old directory] [new directory] [destination file]", argv[0]);
        return 0;
    }

    // Debug configuration doesn't compile without this???
    assert(argc == 4);

    std::filesystem::path oldDirectoryPath = argv[1];
    std::filesystem::path newDirectoryPath = argv[2];

    std::vector<BlockCompressionDiffPatchEntry> entries;
    std::vector<BlockCompressionDiffPatch> patches;
    std::vector<uint8_t> patchBytes;

    for (auto& oldFile : std::filesystem::recursive_directory_iterator(oldDirectoryPath))
    {
        auto newFile = newDirectoryPath / std::filesystem::relative(oldFile, oldDirectoryPath);
        if (!std::filesystem::exists(newFile))
        {
            fprintf(stderr, "Cannot locate %s\n", newFile.string().c_str());
            continue;
        }

        auto oldFileData = readAllBytes(oldFile.path().string().c_str());
        auto newFileData = readAllBytes(newFile.string().c_str());

        constexpr size_t BC_STRIDE = 8;

        if (oldFileData.size() != newFileData.size())
        {
            fprintf(stderr, "%s does not match %s in file size\n", oldFile.path().string().c_str(), newFile.string().c_str());
            continue;
        }

        if ((oldFileData.size() % BC_STRIDE) != 0)
        {
            fprintf(stderr, "%s is not aligned to %d bytes\n", oldFile.path().string().c_str(), BC_STRIDE);
            continue;
        }

        if (oldFileData.size() >= BC_STRIDE)
        {
            size_t patchIndex = patches.size();

            for (size_t i = 0; i < oldFileData.size() - BC_STRIDE + 1; i += BC_STRIDE)
            {
                if (memcmp(&oldFileData[i], &newFileData[i], BC_STRIDE) == 0)
                    continue;

                size_t patchBytesOffset = patchBytes.size();
                patchBytes.insert(patchBytes.end(), newFileData.begin() + i, newFileData.begin() + i + BC_STRIDE);

                if (patchIndex >= patches.size() || ((patches.back().destinationOffset + patches.back().patchBytesSize) != i))
                {
                    auto& patch = patches.emplace_back();
                    patch.destinationOffset = i;
                    patch.patchBytesOffset = patchBytesOffset;
                    patch.patchBytesSize = BC_STRIDE;
                }
                else
                {
                    patches.back().patchBytesSize += BC_STRIDE;
                }
            }

            size_t patchCount = patches.size() - patchIndex;
            if (patchCount != 0)
            {
                auto& entry = entries.emplace_back();
                entry.hash = XXH3_64bits(oldFileData.data(), oldFileData.size());
                entry.patchesOffset = patchIndex * sizeof(BlockCompressionDiffPatch);
                entry.patchCount = patchCount;

                printf("Generated BC patch for %s\n", oldFile.path().string().c_str());
            }
            else
            {
                printf("Skipping %s, files are identical\n", oldFile.path().string().c_str()); 
            }
        }
    }

    std::sort(entries.begin(), entries.end(), [](auto& lhs, auto& rhs) { return lhs.hash < rhs.hash; });

    BlockCompressionDiffPatchHeader header;
    header.entriesOffset = sizeof(BlockCompressionDiffPatchHeader);
    header.entryCount = entries.size();

    size_t patchesOffset = header.entriesOffset + sizeof(BlockCompressionDiffPatchEntry) * entries.size();
    size_t patchBytesOffset = patchesOffset + sizeof(BlockCompressionDiffPatch) * patches.size();

    for (auto& entry : entries)
        entry.patchesOffset += patchesOffset;

    for (auto& patch : patches)
        patch.patchBytesOffset += patchBytesOffset;

    FILE* file = fopen(argv[3], "wb");
    if (!file)
    {
        fprintf(stderr, "Cannot open %s for writing\n", argv[3]);
        return 1;
    }

    fwrite(&header, sizeof(header), 1, file);
    fwrite(entries.data(), sizeof(BlockCompressionDiffPatchEntry), entries.size(), file);
    fwrite(patches.data(), sizeof(BlockCompressionDiffPatch), patches.size(), file);
    fwrite(patchBytes.data(), 1, patchBytes.size(), file);
    fclose(file);

    return 0;
}
