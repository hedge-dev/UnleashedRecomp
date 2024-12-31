#include <stdafx.h>
#include "file_system.h"
#include <kernel/xam.h>
#include <kernel/xdm.h>
#include <kernel/function.h>
#include <cpu/guest_thread.h>
#include <os/logger.h>
#include <mod/mod_loader.h>

struct FileHandle : KernelObject
{
    std::fstream stream;
    std::filesystem::path path;
};

struct FindHandle : KernelObject
{
    std::error_code ec;
    ankerl::unordered_dense::map<std::u8string, std::pair<size_t, bool>> searchResult; // Relative path, file size, is directory
    decltype(searchResult)::iterator iterator;

    FindHandle(const std::string_view& path)
    {
        auto addDirectory = [&](const std::filesystem::path& directory)
            {
                for (auto& entry : std::filesystem::directory_iterator(directory, ec))
                {
                    std::u8string relativePath = entry.path().lexically_relative(directory).u8string();
                    searchResult.emplace(relativePath, std::make_pair(entry.is_directory(ec) ? 0 : entry.file_size(ec), entry.is_directory(ec)));
                }
            };

        std::string_view pathNoPrefix = path;
        size_t index = pathNoPrefix.find(":\\");
        if (index != std::string_view::npos)
            pathNoPrefix.remove_prefix(index + 2);

        // Force add a work folder to let the game see the files in mods,
        // if by some rare chance the user has no DLC or update files.
        if (pathNoPrefix.empty())
            searchResult.emplace(u8"work", std::make_pair(0, true));

        // Look for only work folder in mod folders, AR files cause issues.
        if (pathNoPrefix.starts_with("work"))
        {
            std::string pathStr(pathNoPrefix);
            std::replace(pathStr.begin(), pathStr.end(), '\\', '/');

            for (size_t i = 0; ; i++)
            {
                auto* includeDirs = ModLoader::GetIncludeDirectories(i);
                if (includeDirs == nullptr)
                    break;

                for (auto& includeDir : *includeDirs)
                    addDirectory(includeDir / pathStr);
            }
        }

        addDirectory(FileSystem::ResolvePath(path, false));

        iterator = searchResult.begin();
    }

    void fillFindData(WIN32_FIND_DATAA* lpFindFileData)
    {
        if (iterator->second.second)
            lpFindFileData->dwFileAttributes = ByteSwap(FILE_ATTRIBUTE_DIRECTORY);
        else
            lpFindFileData->dwFileAttributes = ByteSwap(FILE_ATTRIBUTE_NORMAL);

        strncpy(lpFindFileData->cFileName, (const char *)(iterator->first.c_str()), sizeof(lpFindFileData->cFileName));
        lpFindFileData->nFileSizeLow = ByteSwap(uint32_t(iterator->second.first >> 32U));
        lpFindFileData->nFileSizeHigh = ByteSwap(uint32_t(iterator->second.first));
        lpFindFileData->ftCreationTime = {};
        lpFindFileData->ftLastAccessTime = {};
        lpFindFileData->ftLastWriteTime = {};
    }
};

SWA_API FileHandle* XCreateFileA
(
    const char* lpFileName,
    uint32_t dwDesiredAccess,
    uint32_t dwShareMode,
    void* lpSecurityAttributes,
    uint32_t dwCreationDisposition,
    uint32_t dwFlagsAndAttributes
)
{
    assert(((dwDesiredAccess & ~(GENERIC_READ | GENERIC_WRITE | FILE_READ_DATA)) == 0) && "Unknown desired access bits.");
    assert(((dwShareMode & ~(FILE_SHARE_READ | FILE_SHARE_WRITE)) == 0) && "Unknown share mode bits.");
    assert(((dwCreationDisposition & ~(CREATE_NEW | CREATE_ALWAYS)) == 0) && "Unknown creation disposition bits.");

    std::filesystem::path filePath = FileSystem::ResolvePath(lpFileName, true);
    std::fstream fileStream;
    std::ios::openmode fileOpenMode = std::ios::binary;
    if (dwDesiredAccess & (GENERIC_READ | FILE_READ_DATA))
    {
        fileOpenMode |= std::ios::in;
    }

    if (dwDesiredAccess & GENERIC_WRITE)
    {
        fileOpenMode |= std::ios::out;
    }

    fileStream.open(filePath, fileOpenMode);
    if (!fileStream.is_open())
    {
#ifdef _WIN32
        GuestThread::SetLastError(GetLastError());
#endif
        return GetInvalidKernelObject<FileHandle>();
    }

    FileHandle *fileHandle = CreateKernelObject<FileHandle>();
    fileHandle->stream = std::move(fileStream);
    fileHandle->path = std::move(filePath);
    return fileHandle;
}

static uint32_t XGetFileSizeA(FileHandle* hFile, be<uint32_t>* lpFileSizeHigh)
{
    std::error_code ec;
    auto fileSize = std::filesystem::file_size(hFile->path, ec);
    if (!ec)
    {
        if (lpFileSizeHigh != nullptr)
        {
            *lpFileSizeHigh = uint32_t(fileSize >> 32U);
        }
    
        return (uint32_t)(fileSize);
    }

    return INVALID_FILE_SIZE;
}

uint32_t XGetFileSizeExA(FileHandle* hFile, LARGE_INTEGER* lpFileSize)
{
    std::error_code ec;
    auto fileSize = std::filesystem::file_size(hFile->path, ec);
    if (!ec)
    {
        if (lpFileSize != nullptr)
        {
            lpFileSize->QuadPart = ByteSwap(fileSize);
        }

        return TRUE;
    }

    return FALSE;
}

uint32_t XReadFile
(
    FileHandle* hFile,
    void* lpBuffer,
    uint32_t nNumberOfBytesToRead,
    be<uint32_t>* lpNumberOfBytesRead,
    XOVERLAPPED* lpOverlapped
)
{
    uint32_t result = FALSE;
    if (lpOverlapped != nullptr)
    {
        std::streamoff streamOffset = lpOverlapped->Offset + (std::streamoff(lpOverlapped->OffsetHigh.get()) << 32U);
        hFile->stream.clear();
        hFile->stream.seekg(streamOffset, std::ios::beg);
        if (hFile->stream.bad())
        {
            return FALSE;
        }
    }

    uint32_t numberOfBytesRead;
    hFile->stream.read((char *)(lpBuffer), nNumberOfBytesToRead);
    if (!hFile->stream.bad())
    {
        numberOfBytesRead = uint32_t(hFile->stream.gcount());
        result = TRUE;
    }

    if (result)
    {
        if (lpOverlapped != nullptr)
        {
            lpOverlapped->Internal = 0;
            lpOverlapped->InternalHigh = numberOfBytesRead;
        }
        else if (lpNumberOfBytesRead != nullptr)
        {
            *lpNumberOfBytesRead = numberOfBytesRead;
        }
    }

    return result;
}

uint32_t XSetFilePointer(FileHandle* hFile, int32_t lDistanceToMove, be<int32_t>* lpDistanceToMoveHigh, uint32_t dwMoveMethod)
{
    int32_t distanceToMoveHigh = lpDistanceToMoveHigh ? lpDistanceToMoveHigh->get() : 0;
    std::streamoff streamOffset = lDistanceToMove + (std::streamoff(distanceToMoveHigh) << 32U);
    std::fstream::seekdir streamSeekDir = {};
    switch (dwMoveMethod)
    {
    case FILE_BEGIN:
        streamSeekDir = std::ios::beg;
        break;
    case FILE_CURRENT:
        streamSeekDir = std::ios::cur;
        break;
    case FILE_END:
        streamSeekDir = std::ios::end;
        break;
    default:
        assert(false && "Unknown move method.");
        break;
    }

    hFile->stream.clear();
    hFile->stream.seekg(streamOffset, streamSeekDir);
    if (hFile->stream.bad())
    {
        return INVALID_SET_FILE_POINTER;
    }

    std::streampos streamPos = hFile->stream.tellg();
    if (lpDistanceToMoveHigh != nullptr)
        *lpDistanceToMoveHigh = int32_t(streamPos >> 32U);

    return uint32_t(streamPos);
}

uint32_t XSetFilePointerEx(FileHandle* hFile, int32_t lDistanceToMove, LARGE_INTEGER* lpNewFilePointer, uint32_t dwMoveMethod)
{
    std::fstream::seekdir streamSeekDir = {};
    switch (dwMoveMethod)
    {
    case FILE_BEGIN:
        streamSeekDir = std::ios::beg;
        break;
    case FILE_CURRENT:
        streamSeekDir = std::ios::cur;
        break;
    case FILE_END:
        streamSeekDir = std::ios::end;
        break;
    default:
        assert(false && "Unknown move method.");
        break;
    }

    hFile->stream.clear();
    hFile->stream.seekg(lDistanceToMove, streamSeekDir);
    if (hFile->stream.bad())
    {
        return FALSE;
    }

    if (lpNewFilePointer != nullptr)
    {
        lpNewFilePointer->QuadPart = ByteSwap(int64_t(hFile->stream.tellg()));
    }

    return TRUE;
}

FindHandle* XFindFirstFileA(const char* lpFileName, WIN32_FIND_DATAA* lpFindFileData)
{
    std::string_view path = lpFileName;
    if (path.find("\\*") == (path.size() - 2) || path.find("/*") == (path.size() - 2))
    {
        path.remove_suffix(1);
    }
    else if (path.find("\\*.*") == (path.size() - 4) || path.find("/*.*") == (path.size() - 4))
    {
        path.remove_suffix(3);
    }
    else
    {
        assert(!std::filesystem::path(path).has_extension() && "Unknown search pattern.");
    }

    FindHandle findHandle(path);

    if (findHandle.searchResult.empty())
        return GetInvalidKernelObject<FindHandle>();

    findHandle.fillFindData(lpFindFileData);

    return CreateKernelObject<FindHandle>(std::move(findHandle));
}

uint32_t XFindNextFileA(FindHandle* Handle, WIN32_FIND_DATAA* lpFindFileData)
{
    Handle->iterator++;

    if (Handle->iterator == Handle->searchResult.end())
    {
        return FALSE;
    }
    else
    {
        Handle->fillFindData(lpFindFileData);
        return TRUE;
    }
}

uint32_t XReadFileEx(FileHandle* hFile, void* lpBuffer, uint32_t nNumberOfBytesToRead, XOVERLAPPED* lpOverlapped, uint32_t lpCompletionRoutine)
{
    uint32_t result = FALSE;
    uint32_t numberOfBytesRead;
    std::streamoff streamOffset = lpOverlapped->Offset + (std::streamoff(lpOverlapped->OffsetHigh.get()) << 32U);
    hFile->stream.clear();
    hFile->stream.seekg(streamOffset, std::ios::beg);
    if (hFile->stream.bad())
        return FALSE;

    hFile->stream.read((char *)(lpBuffer), nNumberOfBytesToRead);
    if (!hFile->stream.bad())
    {
        numberOfBytesRead = uint32_t(hFile->stream.gcount());
        result = TRUE;
    }

    if (result)
    {
        lpOverlapped->Internal = 0;
        lpOverlapped->InternalHigh = numberOfBytesRead;
    }

    return result;
}

uint32_t XGetFileAttributesA(const char* lpFileName)
{
    std::filesystem::path filePath = FileSystem::ResolvePath(lpFileName, true);
    if (std::filesystem::is_directory(filePath))
        return FILE_ATTRIBUTE_DIRECTORY;
    else if (std::filesystem::is_regular_file(filePath))
        return FILE_ATTRIBUTE_NORMAL;
    else
        return INVALID_FILE_ATTRIBUTES;
}

uint32_t XWriteFile(FileHandle* hFile, const void* lpBuffer, uint32_t nNumberOfBytesToWrite, be<uint32_t>* lpNumberOfBytesWritten, void* lpOverlapped)
{
    assert(lpOverlapped == nullptr && "Overlapped not implemented.");

    hFile->stream.write((const char *)(lpBuffer), nNumberOfBytesToWrite);
    if (hFile->stream.bad())
        return FALSE;

    if (lpNumberOfBytesWritten != nullptr)
        *lpNumberOfBytesWritten = uint32_t(hFile->stream.gcount());

    return TRUE;
}

std::filesystem::path FileSystem::ResolvePath(const std::string_view& path, bool checkForMods)
{
    if (checkForMods)
    {
        std::filesystem::path resolvedPath = ModLoader::ResolvePath(path);
        if (!resolvedPath.empty())
            return resolvedPath;
    }

    thread_local std::string builtPath;
    builtPath.clear();

    size_t index = path.find(":\\");
    if (index != std::string::npos)
    {
        // rooted folder, handle direction
        const std::string_view root = path.substr(0, index);
        const auto newRoot = XamGetRootPath(root);

        if (!newRoot.empty())
        {
            builtPath += newRoot;
            builtPath += '/';
        }
        
        builtPath += path.substr(index + 2);
    }
    else
    {
        builtPath += path;
    }

    std::replace(builtPath.begin(), builtPath.end(), '\\', '/');

    return std::u8string_view((const char8_t*)builtPath.c_str());
}

GUEST_FUNCTION_HOOK(sub_82BD4668, XCreateFileA);
GUEST_FUNCTION_HOOK(sub_82BD4600, XGetFileSizeA);
GUEST_FUNCTION_HOOK(sub_82BD5608, XGetFileSizeExA);
GUEST_FUNCTION_HOOK(sub_82BD4478, XReadFile);
GUEST_FUNCTION_HOOK(sub_831CD3E8, XSetFilePointer);
GUEST_FUNCTION_HOOK(sub_831CE888, XSetFilePointerEx);
GUEST_FUNCTION_HOOK(sub_831CDC58, XFindFirstFileA);
GUEST_FUNCTION_HOOK(sub_831CDC00, XFindNextFileA);
GUEST_FUNCTION_HOOK(sub_831CDF40, XReadFileEx);
GUEST_FUNCTION_HOOK(sub_831CD6E8, XGetFileAttributesA);
GUEST_FUNCTION_HOOK(sub_831CE3F8, XCreateFileA);
GUEST_FUNCTION_HOOK(sub_82BD4860, XWriteFile);
