#include <stdafx.h>
#include <cpu/code_cache.h>
#include <cpu/guest_thread.h>
#include <gpu/video.h>
#include <kernel/function.h>
#include <kernel/memory.h>
#include <kernel/heap.h>
#include <kernel/xam.h>
#include <kernel/io/file_system.h>
#include <file.h>
#include <xex.h>
#include <apu/audio.h>
#include <hid/hid.h>
#include <user/achievement_data.h>
#include <user/config.h>
#include <user/paths.h>
#include <kernel/xdbf.h>
#include <install/installer.h>
#include <ui/installer_wizard.h>

#define GAME_XEX_PATH "game:\\default.xex"

const size_t XMAIOBegin = 0x7FEA0000;
const size_t XMAIOEnd = XMAIOBegin + 0x0000FFFF;

Memory g_memory{ reinterpret_cast<void*>(0x100000000), 0x100000000 };
Heap g_userHeap;
CodeCache g_codeCache;
XDBFWrapper g_xdbfWrapper;
std::unordered_map<uint16_t, GuestTexture*> g_xdbfTextureCache;

void HostStartup()
{
#ifdef _WIN32
    CoInitializeEx(nullptr, COINIT_MULTITHREADED);
#endif

    g_memory.Alloc(0x10000, 0x1000, MEM_COMMIT);
    g_userHeap.Init();
    g_codeCache.Init();

    g_memory.Alloc(XMAIOBegin, 0xFFFF, MEM_COMMIT);

    hid::Init();
}

// Name inspired from nt's entry point
void KiSystemStartup()
{
    const auto gameContent = XamMakeContent(XCONTENTTYPE_RESERVED, "Game");
    const auto updateContent = XamMakeContent(XCONTENTTYPE_RESERVED, "Update");
    XamRegisterContent(gameContent, DirectoryExists(".\\game") ? ".\\game" : ".");
    XamRegisterContent(updateContent, ".\\update");

    const auto savePath = GetSavePath();
    const auto saveName = "SYS-DATA";

    // TODO: implement save slots?
    if (std::filesystem::exists(savePath / saveName))
        XamRegisterContent(XamMakeContent(XCONTENTTYPE_SAVEDATA, saveName), savePath.string());

    // Mount game
    XamContentCreateEx(0, "game", &gameContent, OPEN_EXISTING, nullptr, nullptr, 0, 0, nullptr);
    XamContentCreateEx(0, "update", &updateContent, OPEN_EXISTING, nullptr, nullptr, 0, 0, nullptr);

    // OS mounts game data to D:
    XamContentCreateEx(0, "D", &gameContent, OPEN_EXISTING, nullptr, nullptr, 0, 0, nullptr);

    WIN32_FIND_DATAA fdata;
    const auto findHandle = FindFirstFileA(".\\dlc\\*.*", &fdata);
    if (findHandle != INVALID_HANDLE_VALUE)
    {
        char strBuf[256];
        do
        {
            if (strcmp(fdata.cFileName, ".") == 0 || strcmp(fdata.cFileName, "..") == 0)
            {
                continue;
            }

            if (fdata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                snprintf(strBuf, sizeof(strBuf), ".\\dlc\\%s", fdata.cFileName);
                XamRegisterContent(XamMakeContent(XCONTENTTYPE_DLC, fdata.cFileName), strBuf);
            }
        } while (FindNextFileA(findHandle, &fdata));
        FindClose(findHandle);
    }

    XAudioInitializeSystem();
}

uint32_t LdrLoadModule(const char* path)
{
    auto loadResult = LoadFile(FileSystem::TransformPath(GAME_XEX_PATH));
    if (!loadResult.has_value())
    {
        assert("Failed to load module" && false);
        return 0;
    }

    auto* xex = reinterpret_cast<XEX_HEADER*>(loadResult->data());
    auto security = reinterpret_cast<XEX2_SECURITY_INFO*>((char*)xex + xex->AddressOfSecurityInfo);

    g_memory.Alloc(security->ImageBase, security->SizeOfImage, MEM_COMMIT);

    auto format = Xex2FindOptionalHeader<XEX_FILE_FORMAT_INFO>(xex, XEX_HEADER_FILE_FORMAT_INFO);
    auto entry = *Xex2FindOptionalHeader<uint32_t>(xex, XEX_HEADER_ENTRY_POINT);
    ByteSwap(entry);

    auto srcData = (char *)xex + xex->SizeOfHeader;
    auto destData = (char *)g_memory.Translate(security->ImageBase);
    if (format->CompressionType == 0)
    {
        memcpy(destData, srcData, security->SizeOfImage);
    }
    else if (format->CompressionType == 1)
    {
        auto numBlocks = (format->SizeOfHeader / sizeof(XEX_BASIC_FILE_COMPRESSION_INFO)) - 1;
        auto blocks = reinterpret_cast<const XEX_BASIC_FILE_COMPRESSION_INFO*>(format + 1);

        for (size_t i = 0; i < numBlocks; i++)
        {
            memcpy(destData, srcData, blocks[i].SizeOfData);

            srcData += blocks[i].SizeOfData;
            destData += blocks[i].SizeOfData;
            memset(destData, 0, blocks[i].SizeOfPadding);

            destData += blocks[i].SizeOfPadding;
        }
    }
    else
    {
        assert(false && "Unknown compression type.");
    }

    auto res = Xex2FindOptionalHeader<XEX_RESOURCE_INFO>(xex, XEX_HEADER_RESOURCE_INFO);

    g_xdbfWrapper = XDBFWrapper((uint8_t*)g_memory.Translate(res->Offset.get()), res->SizeOfData);

    return entry;
}

int main(int argc, char *argv[])
{
    bool forceInstaller = false;
    bool forceDLCInstaller = false;
    for (uint32_t i = 1; i < argc; i++)
    {
        forceInstaller = forceInstaller || (strcmp(argv[i], "--install") == 0);
        forceDLCInstaller = forceDLCInstaller || (strcmp(argv[i], "--install-dlc") == 0);
    }

    Config::Load();

    HostStartup();

    Video::CreateHostDevice();

    bool isGameInstalled = Installer::checkGameInstall(".");
    if (forceInstaller || forceDLCInstaller || !isGameInstalled)
    {
        if (!InstallerWizard::Run(isGameInstalled && forceDLCInstaller))
        {
            return 1;
        }
    }

    AchievementData::Load();

    KiSystemStartup();

    uint32_t entry = LdrLoadModule(FileSystem::TransformPath(GAME_XEX_PATH));

    Video::StartPipelinePrecompilation();

    GuestThread::Start(entry);

    return 0;
}

GUEST_FUNCTION_STUB(__imp__vsprintf);
GUEST_FUNCTION_STUB(__imp___vsnprintf);
GUEST_FUNCTION_STUB(__imp__sprintf);
GUEST_FUNCTION_STUB(__imp___snprintf);
GUEST_FUNCTION_STUB(__imp___snwprintf);
GUEST_FUNCTION_STUB(__imp__vswprintf);
GUEST_FUNCTION_STUB(__imp___vscwprintf);
GUEST_FUNCTION_STUB(__imp__swprintf);
