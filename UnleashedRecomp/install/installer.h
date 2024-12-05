#pragma once

#include <span>
#include <set>

#include "virtual_file_system.h"
#include "xex_patcher.h"

enum class DLC {
    Unknown,
    Spagonia,
    Chunnan,
    Mazuri,
    Holoska,
    ApotosShamar,
    EmpireCityAdabat,
    Count = EmpireCityAdabat
};

struct Journal
{
    enum class Result
    {
        Success,
        VirtualFileSystemFailed,
        DirectoryCreationFailed,
        FileMissing,
        FileReadFailed,
        FileHashFailed,
        FileCreationFailed,
        FileWriteFailed,
        ValidationFileMissing,
        DLCParsingFailed,
        PatchProcessFailed,
        PatchReplacementFailed,
        UnknownDLCType
    };

    uint64_t progressCounter = 0;
    uint64_t progressTotal = 0;
    std::list<std::filesystem::path> createdFiles;
    std::set<std::filesystem::path> createdDirectories;
    Result lastResult = Result::Success;
    XexPatcher::Result lastPatcherResult = XexPatcher::Result::Success;
    std::string lastErrorMessage;
};

using FilePair = std::pair<const char *, uint32_t>;

struct Installer
{
    struct Input
    {
        std::filesystem::path gameSource;
        std::filesystem::path updateSource;
        std::list<std::filesystem::path> dlcSources;
    };

    struct DLCSource {
        std::unique_ptr<VirtualFileSystem> sourceVfs;
        std::span<const FilePair> filePairs;
        const uint64_t *fileHashes = nullptr;
        std::string targetSubDirectory;
    };

    struct Sources 
    {
        std::unique_ptr<VirtualFileSystem> game;
        std::unique_ptr<VirtualFileSystem> update;
        std::vector<DLCSource> dlc;
        uint64_t totalSize = 0;
    };

    static bool checkGameInstall(const std::filesystem::path &baseDirectory);
    static bool checkDLCInstall(const std::filesystem::path &baseDirectory, DLC dlc);
    static bool computeTotalSize(std::span<const FilePair> filePairs, const uint64_t *fileHashes, VirtualFileSystem &sourceVfs, Journal &journal, uint64_t &totalSize);
    static bool copyFiles(std::span<const FilePair> filePairs, const uint64_t *fileHashes, VirtualFileSystem &sourceVfs, const std::filesystem::path &targetDirectory, const std::string &validationFile, bool skipHashChecks, Journal &journal, const std::function<void()> &progressCallback);
    static bool parseContent(const std::filesystem::path &sourcePath, std::unique_ptr<VirtualFileSystem> &targetVfs, Journal &journal);
    static bool parseSources(const Input &input, Journal &journal, Sources &sources);
    static bool install(const Sources &sources, const std::filesystem::path &targetDirectory, bool skipHashChecks, Journal &journal, const std::function<void()> &progressCallback);
    static void rollback(Journal &journal);

    // Convenience method for checking if the specified file contains the game. This should be used when the user selects the file.
    static bool parseGame(const std::filesystem::path &sourcePath);

    // Convenience method for checking if the specified file contains the update. This should be used when the user selects the file.
    static bool parseUpdate(const std::filesystem::path &sourcePath);

    // Convenience method for the installer to check which DLC the file that was specified corresponds to. This should be used when the user selects the file.
    static DLC parseDLC(const std::filesystem::path &sourcePath);

    // Convenience method for checking if a game and an update are compatible. This should be used when the user presses next during installation.
    static XexPatcher::Result checkGameUpdateCompatibility(const std::filesystem::path &gameSourcePath, const std::filesystem::path &updateSourcePath);
};
