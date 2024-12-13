#include <stdafx.h>
#include "file_system.h"
#include <kernel/xam.h>
#include <kernel/xdm.h>
#include <kernel/function.h>
#include <cpu/guest_thread.h>

static constexpr uint32_t GUEST_INVALID_HANDLE_VALUE = 0xFFFFFFFF;

struct FileHandle
{
    std::fstream stream;
    std::filesystem::path path;
};

struct FindHandle
{
    std::filesystem::path searchPath;
    std::filesystem::directory_iterator iterator;

    void fillFindData(LPWIN32_FIND_DATAA lpFindFileData)
    {
        if (iterator->is_directory())
            lpFindFileData->dwFileAttributes = std::byteswap(FILE_ATTRIBUTE_DIRECTORY);
        else if (iterator->is_regular_file())
            lpFindFileData->dwFileAttributes = std::byteswap(FILE_ATTRIBUTE_NORMAL);

        std::u8string pathU8Str = std::filesystem::relative(iterator->path(), searchPath).u8string();
        uint64_t fileSize = iterator->file_size();
        strncpy(lpFindFileData->cFileName, (const char *)(pathU8Str.c_str()), sizeof(lpFindFileData->cFileName));
        lpFindFileData->nFileSizeLow = std::byteswap(uint32_t(fileSize >> 32U));
        lpFindFileData->nFileSizeHigh = std::byteswap(uint32_t(fileSize));
        lpFindFileData->ftCreationTime = {};
        lpFindFileData->ftLastAccessTime = {};
        lpFindFileData->ftLastWriteTime = {};
    }
};

bool FileHandleCloser(void* handle)
{
    delete (FileHandle *)(handle);
    return false;
}

bool FindHandleCloser(void* handle)
{
    delete (FindHandle *)(handle);
    return false;
}

SWA_API uint32_t XCreateFileA(
    LPCSTR lpFileName,
    DWORD dwDesiredAccess,
    DWORD dwShareMode,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    DWORD dwCreationDisposition,
    DWORD dwFlagsAndAttributes)
{
    assert(((dwDesiredAccess & ~(GENERIC_READ | GENERIC_WRITE | FILE_READ_DATA)) == 0) && "Unknown desired access bits.");
    assert(((dwShareMode & ~(FILE_SHARE_READ | FILE_SHARE_WRITE)) == 0) && "Unknown share mode bits.");
    assert(((dwCreationDisposition & ~(CREATE_NEW | CREATE_ALWAYS)) == 0) && "Unknown creation disposition bits.");

    std::filesystem::path filePath = FileSystem::TransformPath(lpFileName);
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
        return GUEST_INVALID_HANDLE_VALUE;
    }

    FileHandle *fileHandle = new FileHandle();
    fileHandle->stream = std::move(fileStream);
    fileHandle->path = std::move(filePath);
    return GUEST_HANDLE(ObInsertObject(fileHandle, FileHandleCloser));
}

static DWORD XGetFileSizeA(
    uint32_t hFile,
    LPDWORD lpFileSizeHigh)
{
    FileHandle *handle = (FileHandle *)(ObQueryObject(HOST_HANDLE(hFile)));
    if (handle != nullptr)
    {
        std::error_code ec;
        auto fileSize = std::filesystem::file_size(handle->path, ec);
        if (!ec)
        {
            if (lpFileSizeHigh != nullptr)
            {
                *lpFileSizeHigh = std::byteswap(DWORD(fileSize >> 32U));
            }

            return (DWORD)(fileSize);
        }
    }

    return INVALID_FILE_SIZE;
}

BOOL XGetFileSizeExA(
    uint32_t hFile,
    PLARGE_INTEGER lpFileSize)
{
    FileHandle *handle = (FileHandle *)(ObQueryObject(HOST_HANDLE(hFile)));
    if (handle != nullptr)
    {
        std::error_code ec;
        auto fileSize = std::filesystem::file_size(handle->path, ec);
        if (!ec)
        {
            if (lpFileSize != nullptr)
            {
                lpFileSize->QuadPart = std::byteswap(fileSize);
            }

            return 1;
        }
    }

    return 0;
}

BOOL XReadFile(
    uint32_t hFile,
    LPVOID lpBuffer,
    DWORD nNumberOfBytesToRead,
    XLPDWORD lpNumberOfBytesRead,
    XOVERLAPPED* lpOverlapped)
{
    FileHandle *handle = (FileHandle *)(ObQueryObject(HOST_HANDLE(hFile)));
    if (handle == nullptr)
    {
        return FALSE;
    }

    BOOL result = FALSE;
    if (lpOverlapped != nullptr)
    {
        std::streamoff streamOffset = lpOverlapped->Offset + (std::streamoff(lpOverlapped->OffsetHigh.get()) << 32U);
        handle->stream.clear();
        handle->stream.seekg(streamOffset, std::ios::beg);
        if (handle->stream.bad())
        {
            return FALSE;
        }
    }

    DWORD numberOfBytesRead;
    handle->stream.read((char *)(lpBuffer), nNumberOfBytesToRead);
    if (!handle->stream.bad())
    {
        numberOfBytesRead = DWORD(handle->stream.gcount());
        result = TRUE;
    }

    if (result)
    {
        if (lpOverlapped != nullptr)
        {
            lpOverlapped->Internal = 0;
            lpOverlapped->InternalHigh = numberOfBytesRead;

            if (lpOverlapped->hEvent != NULL)
                SetEvent((HANDLE)lpOverlapped->hEvent.get());
        }
        else if (lpNumberOfBytesRead != nullptr)
        {
            *lpNumberOfBytesRead = numberOfBytesRead;
        }
    }

    return result;
}

DWORD XSetFilePointer(
    uint32_t hFile,
    LONG lDistanceToMove,
    PLONG lpDistanceToMoveHigh,
    DWORD dwMoveMethod)
{
    FileHandle *handle = (FileHandle *)(ObQueryObject(HOST_HANDLE(hFile)));
    if (handle == nullptr)
    {
        return INVALID_SET_FILE_POINTER;
    }

    LONG distanceToMoveHigh = lpDistanceToMoveHigh ? std::byteswap(*lpDistanceToMoveHigh) : 0;
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

    handle->stream.clear();
    handle->stream.seekg(streamOffset, streamSeekDir);
    if (handle->stream.bad())
    {
        return INVALID_SET_FILE_POINTER;
    }

    std::streampos streamPos = handle->stream.tellg();
    if (lpDistanceToMoveHigh != nullptr)
        *lpDistanceToMoveHigh = std::byteswap(LONG(streamPos >> 32U));

    return DWORD(streamPos);
}

BOOL XSetFilePointerEx(
    uint32_t hFile,
    LONG lDistanceToMove,
    PLARGE_INTEGER lpNewFilePointer,
    DWORD dwMoveMethod)
{
    FileHandle *handle = (FileHandle *)(ObQueryObject(HOST_HANDLE(hFile)));
    if (handle == nullptr)
    {
        return FALSE;
    }

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

    handle->stream.clear();
    handle->stream.seekg(lDistanceToMove, streamSeekDir);
    if (handle->stream.bad())
    {
        return FALSE;
    }

    if (lpNewFilePointer != nullptr)
    {
        lpNewFilePointer->QuadPart = std::byteswap(LONGLONG(handle->stream.tellg()));
    }

    return TRUE;
}

uint32_t XFindFirstFileA(
    LPCSTR lpFileName,
    LPWIN32_FIND_DATAA lpFindFileData)
{
    const char *transformedPath = FileSystem::TransformPath(lpFileName);
    size_t transformedPathLength = strlen(transformedPath);
    if (transformedPathLength == 0)
        return GUEST_INVALID_HANDLE_VALUE;

    std::filesystem::path dirPath;
    if (strstr(transformedPath, "\\*") == (&transformedPath[transformedPathLength - 2]))
    {
        dirPath = std::string(transformedPath, transformedPathLength - 2);
    }
    else if (strstr(transformedPath, "\\*.*") == (&transformedPath[transformedPathLength - 4]))
    {
        dirPath = std::string(transformedPath, transformedPathLength - 4);
    }
    else
    {
        dirPath = std::string(transformedPath, transformedPathLength);
        assert(!dirPath.has_extension() && "Unknown search pattern.");
    }

    if (!std::filesystem::is_directory(dirPath))
        return GUEST_INVALID_HANDLE_VALUE;

    std::filesystem::directory_iterator dirIterator(dirPath);
    if (dirIterator == std::filesystem::directory_iterator())
        return GUEST_INVALID_HANDLE_VALUE;

    FindHandle *findHandle = new FindHandle();
    findHandle->searchPath = std::move(dirPath);
    findHandle->iterator = std::move(dirIterator);
    findHandle->fillFindData(lpFindFileData);
    return GUEST_HANDLE(ObInsertObject(findHandle, FindHandleCloser));
}

uint32_t XFindNextFileA(uint32_t Handle, LPWIN32_FIND_DATAA lpFindFileData)
{
    FindHandle *findHandle = (FindHandle *)(ObQueryObject(HOST_HANDLE(Handle)));
    if (findHandle == nullptr)
        return FALSE;

    findHandle->iterator++;

    if (findHandle->iterator == std::filesystem::directory_iterator())
    {
        return FALSE;
    }
    else
    {
        findHandle->fillFindData(lpFindFileData);
        return TRUE;
    }
}

BOOL XReadFileEx(
    uint32_t hFile,
    LPVOID lpBuffer,
    DWORD nNumberOfBytesToRead,
    XOVERLAPPED* lpOverlapped,
    uint32_t lpCompletionRoutine)
{
    FileHandle *handle = (FileHandle *)(ObQueryObject(HOST_HANDLE(hFile)));
    if (handle == nullptr)
        return FALSE;

    BOOL result = FALSE;
    DWORD numberOfBytesRead;
    std::streamoff streamOffset = lpOverlapped->Offset + (std::streamoff(lpOverlapped->OffsetHigh.get()) << 32U);
    handle->stream.clear();
    handle->stream.seekg(streamOffset, std::ios::beg);
    if (handle->stream.bad())
        return FALSE;

    handle->stream.read((char *)(lpBuffer), nNumberOfBytesToRead);
    if (!handle->stream.bad())
    {
        numberOfBytesRead = DWORD(handle->stream.gcount());
        result = TRUE;
    }

    if (result)
    {
        lpOverlapped->Internal = 0;
        lpOverlapped->InternalHigh = numberOfBytesRead;

        if (lpOverlapped->hEvent != NULL)
            SetEvent((HANDLE)lpOverlapped->hEvent.get());
    }

    return result;
}

DWORD XGetFileAttributesA(LPCSTR lpFileName)
{
    std::filesystem::path filePath = FileSystem::TransformPath(lpFileName);
    if (std::filesystem::is_directory(filePath))
        return FILE_ATTRIBUTE_DIRECTORY;
    else if (std::filesystem::is_regular_file(filePath))
        return FILE_ATTRIBUTE_NORMAL;
    else
        return INVALID_FILE_ATTRIBUTES;
}

BOOL XWriteFile(
    uint32_t hFile,
    LPCVOID lpBuffer,
    DWORD nNumberOfBytesToWrite,
    LPDWORD lpNumberOfBytesWritten,
    LPOVERLAPPED lpOverlapped)
{
    assert(lpOverlapped == nullptr && "Overlapped not implemented.");

    FileHandle *handle = (FileHandle *)(ObQueryObject(HOST_HANDLE(hFile)));
    if (handle == nullptr)
        return FALSE;

    handle->stream.write((const char *)(lpBuffer), nNumberOfBytesToWrite);
    if (handle->stream.bad())
        return FALSE;

    if (lpNumberOfBytesWritten != nullptr)
        *lpNumberOfBytesWritten = DWORD(handle->stream.gcount());

    return TRUE;
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

            return builtPath;
        }
    }

    return relativePath != nullptr ? relativePath + 2 : path;
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
