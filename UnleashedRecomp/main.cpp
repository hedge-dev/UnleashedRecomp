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
#include <os/logger.h>
#include <ui/installer_wizard.h>
#include <mod/mod_loader.h>

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
    XamRegisterContent(gameContent, GAME_INSTALL_DIRECTORY "/game");
    XamRegisterContent(updateContent, GAME_INSTALL_DIRECTORY "/update");

    const auto saveFilePath = GetSaveFilePath(true);
    bool saveFileExists = std::filesystem::exists(saveFilePath);

    if (!saveFileExists)
    {
        // Copy base save data to modded save as fallback.
        std::error_code ec;
        std::filesystem::create_directories(saveFilePath.parent_path(), ec);

        if (!ec)
        {
            std::filesystem::copy_file(GetSaveFilePath(false), saveFilePath, ec);
            saveFileExists = !ec;
        }
    }

    if (saveFileExists)
    {
        std::u8string savePathU8 = saveFilePath.parent_path().u8string();
        XamRegisterContent(XamMakeContent(XCONTENTTYPE_SAVEDATA, "SYS-DATA"), (const char*)(savePathU8.c_str()));
    }

    // Mount game
    XamContentCreateEx(0, "game", &gameContent, OPEN_EXISTING, nullptr, nullptr, 0, 0, nullptr);
    XamContentCreateEx(0, "update", &updateContent, OPEN_EXISTING, nullptr, nullptr, 0, 0, nullptr);

    // OS mounts game data to D:
    XamContentCreateEx(0, "D", &gameContent, OPEN_EXISTING, nullptr, nullptr, 0, 0, nullptr);

    std::error_code ec;
    for (auto& file : std::filesystem::directory_iterator(GAME_INSTALL_DIRECTORY "/dlc", ec))
    {
        if (file.is_directory())
        {
            std::u8string fileNameU8 = file.path().filename().u8string();
            std::u8string filePathU8 = file.path().u8string();
            XamRegisterContent(XamMakeContent(XCONTENTTYPE_DLC, (const char*)(fileNameU8.c_str())), (const char*)(filePathU8.c_str()));
        }
    }

    XAudioInitializeSystem();
}

uint32_t LdrLoadModule(const std::filesystem::path &path)
{
    auto loadResult = LoadFile(path);
    if (loadResult.empty())
    {
        assert("Failed to load module" && false);
        return 0;
    }

    auto* xex = reinterpret_cast<XEX_HEADER*>(loadResult.data());
    auto security = reinterpret_cast<XEX2_SECURITY_INFO*>((char*)xex + xex->AddressOfSecurityInfo);

    g_memory.Alloc(security->ImageBase, security->SizeOfImage, MEM_COMMIT);

    auto format = Xex2FindOptionalHeader<XEX_FILE_FORMAT_INFO>(xex, XEX_HEADER_FILE_FORMAT_INFO);
    auto entry = *Xex2FindOptionalHeader<uint32_t>(xex, XEX_HEADER_ENTRY_POINT);
    ByteSwapInplace(entry);

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
#ifdef _WIN32
    timeBeginPeriod(1);
#endif

    os::logger::Init();

    bool forceInstaller = false;
    bool forceDLCInstaller = false;
    const char *sdlVideoDriver = nullptr;
    for (uint32_t i = 1; i < argc; i++)
    {
        forceInstaller = forceInstaller || (strcmp(argv[i], "--install") == 0);
        forceDLCInstaller = forceDLCInstaller || (strcmp(argv[i], "--install-dlc") == 0);
        if (strcmp(argv[i], "--sdl-video-driver") == 0)
        {
            if ((i + 1) < argc)
                sdlVideoDriver = argv[++i];
            else
                fmt::println("No argument was specified for --sdl-video-driver. Option was ignored.");
        }
    }

    Config::Load();

    HostStartup();

    bool isGameInstalled = Installer::checkGameInstall(GAME_INSTALL_DIRECTORY);
    bool runInstallerWizard = forceInstaller || forceDLCInstaller || !isGameInstalled;
    if (runInstallerWizard)
    {
        Video::CreateHostDevice(sdlVideoDriver);

        if (!InstallerWizard::Run(GAME_INSTALL_DIRECTORY, isGameInstalled && forceDLCInstaller))
        {
            return 1;
        }
    }

    ModLoader::Init();

    AchievementData::Load();

    KiSystemStartup();

    auto modulePath = FileSystem::ResolvePath(GAME_XEX_PATH, false);
    uint32_t entry = LdrLoadModule(modulePath);

    if (!runInstallerWizard)
        Video::CreateHostDevice(sdlVideoDriver);

    Video::StartPipelinePrecompilation();

    GuestThread::Start({ entry, 0, 0 });

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
