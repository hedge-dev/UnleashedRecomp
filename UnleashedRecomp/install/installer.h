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
    EmpireCityAdabat
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

    uint32_t progressCounter = 0;
    uint32_t progressTotal = 0;
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
        bool skipHashChecks = false;
    };

    static bool checkGameInstall(const std::filesystem::path &baseDirectory);
    static bool copyFiles(std::span<const FilePair> filePairs, const uint64_t *fileHashes, VirtualFileSystem &sourceVfs, const std::filesystem::path &targetDirectory, const std::string &validationFile, bool skipHashChecks, Journal &journal, const std::function<void(uint32_t)> &progressCallback);
    static bool parseContent(const std::filesystem::path &sourcePath, std::unique_ptr<VirtualFileSystem> &targetVfs, Journal &journal);
    static bool install(const Input &input, const std::filesystem::path &targetDirectory, Journal &journal, const std::function<void(uint32_t)> &progressCallback);
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
