#include <stdafx.h>
#include "file_system.h"
#include <kernel/xam.h>
#include <kernel/xdm.h>
#include <kernel/function.h>
#include <cpu/guest_thread.h>
#include <os/logger.h>

struct FileHandle : KernelObject
{
    std::fstream stream;
    std::filesystem::path path;
};

struct FindHandle : KernelObject
{
    std::error_code ec;
    std::filesystem::path searchPath;
    std::filesystem::directory_iterator iterator;

    void fillFindData(WIN32_FIND_DATAA* lpFindFileData)
    {
        if (iterator->is_directory())
            lpFindFileData->dwFileAttributes = ByteSwap(FILE_ATTRIBUTE_DIRECTORY);
        else if (iterator->is_regular_file())
            lpFindFileData->dwFileAttributes = ByteSwap(FILE_ATTRIBUTE_NORMAL);

        std::u8string pathU8Str = iterator->path().lexically_relative(searchPath).u8string();
        uint64_t fileSize = iterator->file_size(ec);
        strncpy(lpFindFileData->cFileName, (const char *)(pathU8Str.c_str()), sizeof(lpFindFileData->cFileName));
        lpFindFileData->nFileSizeLow = ByteSwap(uint32_t(fileSize >> 32U));
        lpFindFileData->nFileSizeHigh = ByteSwap(uint32_t(fileSize));
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

    std::filesystem::path filePath = std::u8string_view((const char8_t*)(FileSystem::TransformPath(lpFileName)));
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
    const char *transformedPath = FileSystem::TransformPath(lpFileName);
    size_t transformedPathLength = strlen(transformedPath);
    if (transformedPathLength == 0)
        return (FindHandle*)GUEST_INVALID_HANDLE_VALUE;

    std::filesystem::path dirPath;
    if (strstr(transformedPath, "\\*") == (&transformedPath[transformedPathLength - 2]) || strstr(transformedPath, "/*") == (&transformedPath[transformedPathLength - 2]))
    {
        dirPath = std::u8string_view((const char8_t*)(transformedPath), transformedPathLength - 2);
    }
    else if (strstr(transformedPath, "\\*.*") == (&transformedPath[transformedPathLength - 4]) || strstr(transformedPath, "/*.*") == (&transformedPath[transformedPathLength - 4]))
    {
        dirPath = std::u8string_view((const char8_t *)(transformedPath), transformedPathLength - 4);
    }
    else
    {
        dirPath = std::u8string_view((const char8_t *)(transformedPath), transformedPathLength);
        assert(!dirPath.has_extension() && "Unknown search pattern.");
    }

    if (!std::filesystem::is_directory(dirPath))
        return GetInvalidKernelObject<FindHandle>();

    std::filesystem::directory_iterator dirIterator(dirPath);
    if (dirIterator == std::filesystem::directory_iterator())
        return GetInvalidKernelObject<FindHandle>();

    FindHandle *findHandle = CreateKernelObject<FindHandle>();
    findHandle->searchPath = std::move(dirPath);
    findHandle->iterator = std::move(dirIterator);
    findHandle->fillFindData(lpFindFileData);
    return findHandle;
}

uint32_t XFindNextFileA(FindHandle* Handle, WIN32_FIND_DATAA* lpFindFileData)
{
    Handle->iterator++;

    if (Handle->iterator == std::filesystem::directory_iterator())
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
    std::filesystem::path filePath(std::u8string_view((const char8_t*)(FileSystem::TransformPath(lpFileName))));
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

static void fixSlashes(char *path)
{
    while (*path != 0)
    {
        if (*path == '\\')
        {
            *path = '/';
        }

        path++;
    }
}

const char* FileSystem::TransformPath(const char* path)
{
    thread_local char builtPath[2048]{};
    const char* relativePath = strstr(path, ":\\");
    if (relativePath != nullptr)
    {
        // rooted folder, handle direction
        const std::string_view root = std::string_view{ path, path + (relativePath - path) };
        const auto newRoot = XamGetRootPath(root);

        if (!newRoot.empty())
        {
            strncpy(builtPath, newRoot.data(), newRoot.size());
            builtPath[newRoot.size()] = '\\';
            strcpy(builtPath + newRoot.size() + 1, relativePath + 2);
        }
        else
        {
            strncpy(builtPath, relativePath + 2, sizeof(builtPath));
        }
    }
    else
    {
        strncpy(builtPath, path, sizeof(builtPath));
    }

    fixSlashes(builtPath);
    return builtPath;
}

SWA_API const char* XExpandFilePathA(const char* path)
{
    return FileSystem::TransformPath(path);
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
