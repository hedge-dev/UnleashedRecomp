#include "installer.h"

#include <xxh3.h>

#include "directory_file_system.h"
#include "iso_file_system.h"
#include "xcontent_file_system.h"

#include "hashes/apotos_shamar.h"
#include "hashes/chunnan.h"
#include "hashes/empire_city_adabat.h"
#include "hashes/game.h"
#include "hashes/holoska.h"
#include "hashes/mazuri.h"
#include "hashes/spagonia.h"
#include "hashes/update.h"

static const std::string GameDirectory = "game";
static const std::string DLCDirectory = "dlc";
static const std::string PatchedDirectory = "patched";
static const std::string ApotosShamarDirectory = DLCDirectory + "/Apotos & Shamar Adventure Pack";
static const std::string ChunnanDirectory = DLCDirectory + "/Chun-nan Adventure Pack";
static const std::string EmpireCityAdabatDirectory = DLCDirectory + "/Empire City & Adabat Adventure Pack";
static const std::string HoloskaDirectory = DLCDirectory + "/Holoska Adventure Pack";
static const std::string MazuriDirectory = DLCDirectory + "/Mazuri Adventure Pack";
static const std::string SpagoniaDirectory = DLCDirectory + "/Spagonia Adventure Pack";
static const std::string UpdateDirectory = "update";
static const std::string GameExecutableFile = "default.xex";
static const std::string DLCValidationFile = "DLC.xml";
static const std::string UpdateExecutablePatchFile = "default.xexp";
static const std::string ISOExtension = ".iso";
static const std::string OldExtension = ".old";
static const std::string TempExtension = ".tmp";

static std::string fromU8(const std::u8string &str)
{
    return std::string(str.begin(), str.end());
}

static std::string fromPath(const std::filesystem::path &path)
{
    return fromU8(path.u8string());
}

static std::string toLower(std::string str) {
    std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c) { return std::tolower(c); });
    return str;
};

static std::unique_ptr<VirtualFileSystem> createFileSystemFromPath(const std::filesystem::path &path)
{
    if (XContentFileSystem::check(path))
    {
        return XContentFileSystem::create(path);
    }
    else if (toLower(fromPath(path.extension())) == ISOExtension)
    {
        return ISOFileSystem::create(path);
    }
    else if (std::filesystem::is_directory(path))
    {
        return DirectoryFileSystem::create(path);
    }
    else
    {
        return nullptr;
    }
}

static bool copyFile(const FilePair &pair, const uint64_t *fileHashes, VirtualFileSystem &sourceVfs, const std::filesystem::path &targetDirectory, bool skipHashChecks, std::vector<uint8_t> &fileData, Journal &journal, const std::function<bool()> &progressCallback) {
    const std::string filename(pair.first);
    const uint32_t hashCount = pair.second;
    if (!sourceVfs.exists(filename))
    {
        journal.lastResult = Journal::Result::FileMissing;
        journal.lastErrorMessage = fmt::format("File {} does not exist in {}.", filename, sourceVfs.getName());
        return false;
    }

    if (!sourceVfs.load(filename, fileData))
    {
        journal.lastResult = Journal::Result::FileReadFailed;
        journal.lastErrorMessage = fmt::format("Failed to read file {} from {}.", filename, sourceVfs.getName());
        return false;
    }

    if (!skipHashChecks)
    {
        uint64_t fileHash = XXH3_64bits(fileData.data(), fileData.size());
        bool fileHashFound = false;
        for (uint32_t i = 0; i < hashCount && !fileHashFound; i++)
        {
            fileHashFound = fileHash == fileHashes[i];
        }

        if (!fileHashFound)
        {
            journal.lastResult = Journal::Result::FileHashFailed;
            journal.lastErrorMessage = fmt::format("File {} from {} did not match any of the known hashes.", filename, sourceVfs.getName());
            return false;
        }
    }

    std::filesystem::path targetPath = targetDirectory / std::filesystem::path(std::u8string_view((const char8_t *)(pair.first)));
    std::filesystem::path parentPath = targetPath.parent_path();
    if (!std::filesystem::exists(parentPath))
    {
        std::error_code ec;
        std::filesystem::create_directories(parentPath, ec);
    }
    
    while (!parentPath.empty()) {
        journal.createdDirectories.insert(parentPath);

        if (parentPath != targetDirectory) {
            parentPath = parentPath.parent_path();
        }
        else {
            parentPath = std::filesystem::path();
        }
    }

    std::ofstream outStream(targetPath, std::ios::binary);
    if (!outStream.is_open())
    {
        journal.lastResult = Journal::Result::FileCreationFailed;
        journal.lastErrorMessage = fmt::format("Failed to create file at {}.", fromPath(targetPath));
        return false;
    }

    journal.createdFiles.push_back(targetPath);

    outStream.write((const char *)(fileData.data()), fileData.size());
    if (outStream.bad())
    {
        journal.lastResult = Journal::Result::FileWriteFailed;
        journal.lastErrorMessage = fmt::format("Failed to create file at {}.", fromPath(targetPath));
        return false;
    }

    journal.progressCounter += fileData.size();
    
    if (!progressCallback())
    {
        journal.lastResult = Journal::Result::Cancelled;
        journal.lastErrorMessage = "Installation was cancelled.";
        return false;
    }

    return true;
}

static DLC detectDLC(const std::filesystem::path &sourcePath, VirtualFileSystem &sourceVfs, Journal &journal)
{
    std::vector<uint8_t> dlcXmlBytes;
    if (!sourceVfs.load(DLCValidationFile, dlcXmlBytes))
    {
        journal.lastResult = Journal::Result::FileMissing;
        journal.lastErrorMessage = fmt::format("File {} does not exist in {}.", DLCValidationFile, sourceVfs.getName());
        return DLC::Unknown;
    }

    const char TypeStartString[] = "<Type>";
    const char TypeEndString[] = "</Type>";
    size_t dlcByteCount = dlcXmlBytes.size();
    dlcXmlBytes.resize(dlcByteCount + 1);
    dlcXmlBytes[dlcByteCount] = '\0';
    const char *typeStartLocation = strstr((const char *)(dlcXmlBytes.data()), TypeStartString);
    const char *typeEndLocation = typeStartLocation != nullptr ? strstr(typeStartLocation, TypeEndString) : nullptr;
    if (typeStartLocation == nullptr || typeEndLocation == nullptr)
    {
        journal.lastResult = Journal::Result::DLCParsingFailed;
        journal.lastErrorMessage = fmt::format("Failed to find DLC type for {}.", sourceVfs.getName());
        return DLC::Unknown;
    }

    const char *typeNumberLocation = typeStartLocation + strlen(TypeStartString);
    size_t typeNumberCount = typeEndLocation - typeNumberLocation;
    if (typeNumberCount != 1)
    {
        journal.lastResult = Journal::Result::UnknownDLCType;
        journal.lastErrorMessage = fmt::format("DLC type for {} is unknown.", sourceVfs.getName());
        return DLC::Unknown;
    }

    switch (*typeNumberLocation)
    {
    case '1':
        return DLC::Spagonia;
    case '2':
        return DLC::Chunnan;
    case '3':
        return DLC::Mazuri;
    case '4':
        return DLC::Holoska;
    case '5':
        return DLC::ApotosShamar;
    case '7':
        return DLC::EmpireCityAdabat;
    default:
        journal.lastResult = Journal::Result::UnknownDLCType;
        journal.lastErrorMessage = fmt::format("DLC type for {} is unknown.", sourceVfs.getName());
        return DLC::Unknown;
    }
}

bool Installer::checkGameInstall(const std::filesystem::path &baseDirectory, std::filesystem::path &modulePath)
{
    modulePath = baseDirectory / PatchedDirectory / GameExecutableFile;

    if (!std::filesystem::exists(modulePath))
        return false;

    if (!std::filesystem::exists(baseDirectory / UpdateDirectory / UpdateExecutablePatchFile))
        return false;

    if (!std::filesystem::exists(baseDirectory / GameDirectory / GameExecutableFile))
        return false;

    return true;
}

bool Installer::checkDLCInstall(const std::filesystem::path &baseDirectory, DLC dlc)
{
    switch (dlc)
    {
    case DLC::Spagonia:
        return std::filesystem::exists(baseDirectory / SpagoniaDirectory / DLCValidationFile);
    case DLC::Chunnan:
        return std::filesystem::exists(baseDirectory / ChunnanDirectory / DLCValidationFile);
    case DLC::Mazuri:
        return std::filesystem::exists(baseDirectory / MazuriDirectory / DLCValidationFile);
    case DLC::Holoska:
        return std::filesystem::exists(baseDirectory / HoloskaDirectory / DLCValidationFile);
    case DLC::ApotosShamar:
        return std::filesystem::exists(baseDirectory / ApotosShamarDirectory / DLCValidationFile);
    case DLC::EmpireCityAdabat:
        return std::filesystem::exists(baseDirectory / EmpireCityAdabatDirectory / DLCValidationFile);
    default:
        return false;
    }
}

bool Installer::checkAllDLC(const std::filesystem::path& baseDirectory)
{
    bool result = true;

    for (int i = 1; i < (int)DLC::Count; i++)
    {
        if (!checkDLCInstall(baseDirectory, (DLC)i))
            result = false;
    }

    return result;
}

bool Installer::computeTotalSize(std::span<const FilePair> filePairs, const uint64_t *fileHashes, VirtualFileSystem &sourceVfs, Journal &journal, uint64_t &totalSize)
{
    for (FilePair pair : filePairs)
    {
        const std::string filename(pair.first);
        if (!sourceVfs.exists(filename))
        {
            journal.lastResult = Journal::Result::FileMissing;
            journal.lastErrorMessage = fmt::format("File {} does not exist in {}.", filename, sourceVfs.getName());
            return false;
        }

        totalSize += sourceVfs.getSize(filename);
    }

    return true;
}

bool Installer::copyFiles(std::span<const FilePair> filePairs, const uint64_t *fileHashes, VirtualFileSystem &sourceVfs, const std::filesystem::path &targetDirectory, const std::string &validationFile, bool skipHashChecks, Journal &journal, const std::function<bool()> &progressCallback)
{
    std::error_code ec;
    if (!std::filesystem::exists(targetDirectory) && !std::filesystem::create_directories(targetDirectory, ec))
    {
        journal.lastResult = Journal::Result::DirectoryCreationFailed;
        journal.lastErrorMessage = "Unable to create directory at " + fromPath(targetDirectory);
        return false;
    }

    FilePair validationPair = {};
    uint32_t validationHashIndex = 0;
    uint32_t hashIndex = 0;
    uint32_t hashCount = 0;
    std::vector<uint8_t> fileData;
    for (FilePair pair : filePairs)
    {
        hashIndex = hashCount;
        hashCount += pair.second;

        if (validationFile.compare(pair.first) == 0)
        {
            validationPair = pair;
            validationHashIndex = hashIndex;
            continue;
        }

        if (!copyFile(pair, &fileHashes[hashIndex], sourceVfs, targetDirectory, skipHashChecks, fileData, journal, progressCallback))
        {
            return false;
        }
    }

    // Validation file is copied last after all other files have been copied.
    if (validationPair.first != nullptr)
    {
        if (!copyFile(validationPair, &fileHashes[validationHashIndex], sourceVfs, targetDirectory, skipHashChecks, fileData, journal, progressCallback))
        {
            return false;
        }
    }
    else
    {
        journal.lastResult = Journal::Result::ValidationFileMissing;
        journal.lastErrorMessage = fmt::format("Unable to find validation file {} in {}.", validationFile, sourceVfs.getName());
        return false;
    }

    return true;
}

bool Installer::parseContent(const std::filesystem::path &sourcePath, std::unique_ptr<VirtualFileSystem> &targetVfs, Journal &journal)
{
    targetVfs = createFileSystemFromPath(sourcePath);
    if (targetVfs != nullptr)
    {
        return true;
    }
    else
    {
        journal.lastResult = Journal::Result::VirtualFileSystemFailed;
        journal.lastErrorMessage = "Unable to open " + fromPath(sourcePath);
        return false;
    }
}

constexpr uint32_t PatcherContribution = 512 * 1024 * 1024;

bool Installer::parseSources(const Input &input, Journal &journal, Sources &sources)
{
    journal = Journal();
    sources = Sources();

    // Parse the contents of the base game.
    if (!input.gameSource.empty())
    {
        if (!parseContent(input.gameSource, sources.game, journal))
        {
            return false;
        }

        if (!computeTotalSize({ GameFiles, GameFilesSize }, GameHashes, *sources.game, journal, sources.totalSize))
        {
            return false;
        }
    }

    // Parse the contents of Update.
    if (!input.updateSource.empty())
    {
        // Add an arbitrary progress size for the patching process.
        journal.progressTotal += PatcherContribution;

        if (!parseContent(input.updateSource, sources.update, journal))
        {
            return false;
        }

        if (!computeTotalSize({ UpdateFiles, UpdateFilesSize }, UpdateHashes, *sources.update, journal, sources.totalSize))
        {
            return false;
        }
    }

    // Parse the contents of the DLC Packs.
    for (const auto &path : input.dlcSources)
    {
        sources.dlc.emplace_back();
        DLCSource &dlcSource = sources.dlc.back();
        if (!parseContent(path, dlcSource.sourceVfs, journal))
        {
            return false;
        }

        DLC dlc = detectDLC(path, *dlcSource.sourceVfs, journal);
        switch (dlc)
        {
        case DLC::Spagonia:
            dlcSource.filePairs = { SpagoniaFiles, SpagoniaFilesSize };
            dlcSource.fileHashes = SpagoniaHashes;
            dlcSource.targetSubDirectory = SpagoniaDirectory;
            break;
        case DLC::Chunnan:
            dlcSource.filePairs = { ChunnanFiles, ChunnanFilesSize };
            dlcSource.fileHashes = ChunnanHashes;
            dlcSource.targetSubDirectory = ChunnanDirectory;
            break;
        case DLC::Mazuri:
            dlcSource.filePairs = { MazuriFiles, MazuriFilesSize };
            dlcSource.fileHashes = MazuriHashes;
            dlcSource.targetSubDirectory = MazuriDirectory;
            break;
        case DLC::Holoska:
            dlcSource.filePairs = { HoloskaFiles, HoloskaFilesSize };
            dlcSource.fileHashes = HoloskaHashes;
            dlcSource.targetSubDirectory = HoloskaDirectory;
            break;
        case DLC::ApotosShamar:
            dlcSource.filePairs = { ApotosShamarFiles, ApotosShamarFilesSize };
            dlcSource.fileHashes = ApotosShamarHashes;
            dlcSource.targetSubDirectory = ApotosShamarDirectory;
            break;
        case DLC::EmpireCityAdabat:
            dlcSource.filePairs = { EmpireCityAdabatFiles, EmpireCityAdabatFilesSize };
            dlcSource.fileHashes = EmpireCityAdabatHashes;
            dlcSource.targetSubDirectory = EmpireCityAdabatDirectory;
            break;
        default:
            return false;
        }

        if (!computeTotalSize(dlcSource.filePairs, dlcSource.fileHashes, *dlcSource.sourceVfs, journal, sources.totalSize))
        {
            return false;
        }
    }

    // Add the total size in bytes as the journal progress.
    journal.progressTotal += sources.totalSize;

    return true;
}

bool Installer::install(const Sources &sources, const std::filesystem::path &targetDirectory, bool skipHashChecks, Journal &journal, std::chrono::seconds endWaitTime, const std::function<bool()> &progressCallback)
{
    // Install files in reverse order of importance. In case of a process crash or power outage, this will increase the likelihood of the installation
    // missing critical files required for the game to run. These files are used as the way to detect if the game is installed.

    // Install the DLC.
    if (!sources.dlc.empty())
    {
        journal.createdDirectories.insert(targetDirectory / DLCDirectory);
    }

    for (const DLCSource &dlcSource : sources.dlc)
    {
        if (!copyFiles(dlcSource.filePairs, dlcSource.fileHashes, *dlcSource.sourceVfs, targetDirectory / dlcSource.targetSubDirectory, DLCValidationFile, skipHashChecks, journal, progressCallback))
        {
            return false;
        }
    }

    // If no game or update was specified, we're finished. This means the user was only installing the DLC.
    if ((sources.game == nullptr) && (sources.update == nullptr))
    {
        return true;
    }

    // Install the update.
    if (!copyFiles({ UpdateFiles, UpdateFilesSize }, UpdateHashes, *sources.update, targetDirectory / UpdateDirectory, UpdateExecutablePatchFile, skipHashChecks, journal, progressCallback))
    {
        return false;
    }

    // Install the base game.
    if (!copyFiles({ GameFiles, GameFilesSize }, GameHashes, *sources.game, targetDirectory / GameDirectory, GameExecutableFile, skipHashChecks, journal, progressCallback))
    {
        return false;
    }

    // Create the directory where the patched executable will be stored.
    std::error_code ec;
    std::filesystem::path patchedDirectory = targetDirectory / PatchedDirectory;
    if (!std::filesystem::exists(patchedDirectory) && !std::filesystem::create_directories(patchedDirectory, ec))
    {
        journal.lastResult = Journal::Result::DirectoryCreationFailed;
        journal.lastErrorMessage = "Unable to create directory at " + fromPath(patchedDirectory);
        return false;
    }

    journal.createdDirectories.insert(patchedDirectory);

    // Patch the executable with the update's file.
    std::filesystem::path baseXexPath = targetDirectory / GameDirectory / GameExecutableFile;
    std::filesystem::path patchPath = targetDirectory / UpdateDirectory / UpdateExecutablePatchFile;
    std::filesystem::path patchedXexPath = patchedDirectory / GameExecutableFile;
    XexPatcher::Result patcherResult = XexPatcher::apply(baseXexPath, patchPath, patchedXexPath);
    if (patcherResult == XexPatcher::Result::Success)
    {
        journal.createdFiles.push_back(patchedXexPath);
    }
    else
    {
        journal.lastResult = Journal::Result::PatchProcessFailed;
        journal.lastPatcherResult = patcherResult;
        journal.lastErrorMessage = "Patch process failed.";
        return false;
    }

    // Update the progress with the artificial amount attributed to the patching.
    journal.progressCounter += PatcherContribution;
    
    for (uint32_t i = 0; i < 2; i++)
    {
        if (!progressCallback())
        {
            journal.lastResult = Journal::Result::Cancelled;
            journal.lastErrorMessage = "Installation was cancelled.";
            return false;
        }

        if (i == 0)
        {
            // Wait the specified amount of time to allow the consumer of the callbacks to animate, halt or cancel the installation for a while after it's finished.
            std::this_thread::sleep_for(endWaitTime);
        }
    }

    return true;
}

void Installer::rollback(Journal &journal)
{
    std::error_code ec;
    for (const auto &path : journal.createdFiles)
    {
        std::filesystem::remove(path, ec);
    }

    for (auto it = journal.createdDirectories.rbegin(); it != journal.createdDirectories.rend(); it++)
    {
        std::filesystem::remove(*it, ec);
    }
}

bool Installer::parseGame(const std::filesystem::path &sourcePath)
{
    std::unique_ptr<VirtualFileSystem> sourceVfs = createFileSystemFromPath(sourcePath);
    if (sourceVfs == nullptr)
    {
        return false;
    }

    return sourceVfs->exists(GameExecutableFile);
}

bool Installer::parseUpdate(const std::filesystem::path &sourcePath)
{
    std::unique_ptr<VirtualFileSystem> sourceVfs = createFileSystemFromPath(sourcePath);
    if (sourceVfs == nullptr)
    {
        return false;
    }

    return sourceVfs->exists(UpdateExecutablePatchFile);
}

DLC Installer::parseDLC(const std::filesystem::path &sourcePath)
{
    Journal journal;
    std::unique_ptr<VirtualFileSystem> sourceVfs = createFileSystemFromPath(sourcePath);
    if (sourceVfs == nullptr)
    {
        return DLC::Unknown;
    }

    return detectDLC(sourcePath, *sourceVfs, journal);
}

XexPatcher::Result Installer::checkGameUpdateCompatibility(const std::filesystem::path &gameSourcePath, const std::filesystem::path &updateSourcePath)
{
    std::unique_ptr<VirtualFileSystem> gameSourceVfs = createFileSystemFromPath(gameSourcePath);
    if (gameSourceVfs == nullptr)
    {
        return XexPatcher::Result::FileOpenFailed;
    }

    std::unique_ptr<VirtualFileSystem> updateSourceVfs = createFileSystemFromPath(updateSourcePath);
    if (updateSourceVfs == nullptr)
    {
        return XexPatcher::Result::FileOpenFailed;
    }

    std::vector<uint8_t> xexBytes;
    std::vector<uint8_t> patchBytes;
    if (!gameSourceVfs->load(GameExecutableFile, xexBytes))
    {
        return XexPatcher::Result::FileOpenFailed;
    }

    if (!updateSourceVfs->load(UpdateExecutablePatchFile, patchBytes))
    {
        return XexPatcher::Result::FileOpenFailed;
    }

    std::vector<uint8_t> patchedBytes;
    return XexPatcher::apply(xexBytes.data(), xexBytes.size(), patchBytes.data(), patchBytes.size(), patchedBytes, true);
}
