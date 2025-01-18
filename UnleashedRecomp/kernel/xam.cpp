#include <stdafx.h>
#include "xam.h"
#include "xdm.h"
#include <hid/hid.h>
#include <hid/hid_detail.h>
#include <ui/game_window.h>
#include <cpu/guest_thread.h>
#include <ranges>
#include <unordered_set>
#include "xxHashMap.h"
#include <user/paths.h>
#include <SDL.h>

struct XamListener : KernelObject
{
    uint32_t id{};
    uint64_t areas{};
    std::vector<std::tuple<uint32_t, uint32_t>> notifications;

    XamListener(const XamListener&) = delete;
    XamListener& operator=(const XamListener&) = delete;

    XamListener();
    ~XamListener();
};

struct XamEnumeratorBase : KernelObject
{
    virtual uint32_t Next(void* buffer)
    {
        return -1;
    }
};

template<typename TIterator = std::vector<XHOSTCONTENT_DATA>::iterator>
struct XamEnumerator : XamEnumeratorBase
{
    uint32_t fetch;
    size_t size;
    TIterator position;
    TIterator begin;
    TIterator end;

    XamEnumerator() = default;
    XamEnumerator(uint32_t fetch, size_t size, TIterator begin, TIterator end) : fetch(fetch), size(size), position(begin), begin(begin), end(end)
    {

    }

    uint32_t Next(void* buffer) override
    {
        if (position == end)
        {
            return -1;
        }

        if (buffer == nullptr)
        {
            for (size_t i = 0; i < fetch; i++)
            {
                if (position == end)
                {
                    return i == 0 ? -1 : i;
                }

                ++position;
            }
        }

        for (size_t i = 0; i < fetch; i++)
        {
            if (position == end)
            {
                return i == 0 ? -1 : i;
            }

            memcpy(buffer, &*position, size);

            ++position;
            buffer = (void*)((size_t)buffer + size);
        }

        return fetch;
    }
};

std::array<xxHashMap<XHOSTCONTENT_DATA>, 3> gContentRegistry{};
std::unordered_set<XamListener*> gListeners{};
xxHashMap<std::string> gRootMap;

std::string_view XamGetRootPath(const std::string_view& root)
{
    const auto result = gRootMap.find(StringHash(root));

    if (result == gRootMap.end())
        return "";

    return result->second;
}

void XamRootCreate(const std::string_view& root, const std::string_view& path)
{
    gRootMap.emplace(StringHash(root), path);
}

XamListener::XamListener()
{
    gListeners.insert(this);
}

XamListener::~XamListener()
{
    gListeners.erase(this);
}

XCONTENT_DATA XamMakeContent(uint32_t type, const std::string_view& name)
{
    XCONTENT_DATA data{ 1, type };

    strncpy(data.szFileName, name.data(), sizeof(data.szFileName));

    return data;
}

void XamRegisterContent(const XCONTENT_DATA& data, const std::string_view& root)
{
    const auto idx = data.dwContentType - 1;

    gContentRegistry[idx].emplace(StringHash(data.szFileName), XHOSTCONTENT_DATA{ data }).first->second.szRoot = root;
}

void XamRegisterContent(uint32_t type, const std::string_view name, const std::string_view& root)
{
    XCONTENT_DATA data{ 1, type, {}, "" };

    strncpy(data.szFileName, name.data(), sizeof(data.szFileName));

    XamRegisterContent(data, root);
}

uint32_t XamNotifyCreateListener(uint64_t qwAreas)
{
    auto* listener = CreateKernelObject<XamListener>();

    listener->areas = qwAreas;

    return GetKernelHandle(listener);
}

void XamNotifyEnqueueEvent(uint32_t dwId, uint32_t dwParam)
{
    for (const auto& listener : gListeners)
    {
        if (((1 << MSG_AREA(dwId)) & listener->areas) == 0)
            continue;

        listener->notifications.emplace_back(dwId, dwParam);
    }
}

bool XNotifyGetNext(uint32_t hNotification, uint32_t dwMsgFilter, be<uint32_t>* pdwId, be<uint32_t>* pParam)
{
    auto& listener = *GetKernelObject<XamListener>(hNotification);

    if (dwMsgFilter)
    {
        for (size_t i = 0; i < listener.notifications.size(); i++)
        {
            if (std::get<0>(listener.notifications[i]) == dwMsgFilter)
            {
                if (pdwId)
                    *pdwId = std::get<0>(listener.notifications[i]);

                if (pParam)
                    *pParam = std::get<1>(listener.notifications[i]);

                listener.notifications.erase(listener.notifications.begin() + i);

                return true;
            }
        }
    }
    else
    {
        if (listener.notifications.empty())
            return false;

        if (pdwId)
            *pdwId = std::get<0>(listener.notifications[0]);

        if (pParam)
            *pParam = std::get<1>(listener.notifications[0]);

        listener.notifications.erase(listener.notifications.begin());

        return true;
    }

    return false;
}

uint32_t XamShowMessageBoxUI(uint32_t dwUserIndex, be<uint16_t>* wszTitle, be<uint16_t>* wszText, uint32_t cButtons,
    xpointer<be<uint16_t>>* pwszButtons, uint32_t dwFocusButton, uint32_t dwFlags, be<uint32_t>* pResult, XXOVERLAPPED* pOverlapped)
{
    *pResult = cButtons ? cButtons - 1 : 0;

#if _DEBUG
    assert("XamShowMessageBoxUI encountered!" && false);
#else
    std::vector<std::wstring> texts{};

    texts.emplace_back(reinterpret_cast<wchar_t*>(wszTitle));
    texts.emplace_back(reinterpret_cast<wchar_t*>(wszText));

    for (size_t i = 0; i < cButtons; i++)
        texts.emplace_back(reinterpret_cast<wchar_t*>(pwszButtons[i].get()));

    for (auto& text : texts)
    {
        for (size_t i = 0; i < text.size(); i++)
            ByteSwapInplace(text[i]);
    }

    wprintf(L"[XamShowMessageBoxUI] !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
    wprintf(L"[XamShowMessageBoxUI] If you are encountering this message and the game has ceased functioning,\n");
    wprintf(L"[XamShowMessageBoxUI] please create an issue at https://github.com/hedge-dev/UnleashedRecomp/issues.\n");
    wprintf(L"[XamShowMessageBoxUI] !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
    wprintf(L"[XamShowMessageBoxUI] %ls\n", texts[0].c_str());
    wprintf(L"[XamShowMessageBoxUI] %ls\n", texts[1].c_str());
    wprintf(L"[XamShowMessageBoxUI] ");

    for (size_t i = 0; i < cButtons; i++)
    {
        wprintf(L"%ls", texts[2 + i].c_str());

        if (i != cButtons - 1)
            wprintf(L" | ");
    }

    wprintf(L"\n");
    wprintf(L"[XamShowMessageBoxUI] Defaulted to button: %d\n", pResult->get());
    wprintf(L"[XamShowMessageBoxUI] !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
#endif

    if (pOverlapped)
    {
        pOverlapped->dwCompletionContext = GuestThread::GetCurrentThreadId();
        pOverlapped->Error = 0;
        pOverlapped->Length = -1;
    }

    XamNotifyEnqueueEvent(9, 0);

    return 0;
}

uint32_t XamContentCreateEnumerator(uint32_t dwUserIndex, uint32_t DeviceID, uint32_t dwContentType,
    uint32_t dwContentFlags, uint32_t cItem, be<uint32_t>* pcbBuffer, be<uint32_t>* phEnum)
{
    if (dwUserIndex != 0)
    {
        GuestThread::SetLastError(ERROR_NO_SUCH_USER);
        return 0xFFFFFFFF;
    }

    const auto& registry = gContentRegistry[dwContentType - 1];
    const auto& values = registry | std::views::values;
    auto* enumerator = CreateKernelObject<XamEnumerator<decltype(values.begin())>>(cItem, sizeof(_XCONTENT_DATA), values.begin(), values.end());

    if (pcbBuffer)
        *pcbBuffer = sizeof(_XCONTENT_DATA) * cItem;

    *phEnum = GetKernelHandle(enumerator);

    return 0;
}

uint32_t XamEnumerate(uint32_t hEnum, uint32_t dwFlags, void* pvBuffer, uint32_t cbBuffer, be<uint32_t>* pcItemsReturned, XXOVERLAPPED* pOverlapped)
{
    auto* enumerator = GetKernelObject<XamEnumeratorBase>(hEnum);
    const auto count = enumerator->Next(pvBuffer);

    if (count == -1)
        return ERROR_NO_MORE_FILES;

    if (pcItemsReturned)
        *pcItemsReturned = count;

    return ERROR_SUCCESS;
}

uint32_t XamContentCreateEx(uint32_t dwUserIndex, const char* szRootName, const XCONTENT_DATA* pContentData,
    uint32_t dwContentFlags, be<uint32_t>* pdwDisposition, be<uint32_t>* pdwLicenseMask,
    uint32_t dwFileCacheSize, uint64_t uliContentSize, PXXOVERLAPPED pOverlapped)
{
    const auto& registry = gContentRegistry[pContentData->dwContentType - 1];
    const auto exists = registry.contains(StringHash(pContentData->szFileName));
    const auto mode = dwContentFlags & 0xF;

    if (mode == CREATE_ALWAYS)
    {
        if (pdwDisposition)
            *pdwDisposition = XCONTENT_NEW;

        if (!exists)
        {
            std::string root = "";

            if (pContentData->dwContentType == XCONTENTTYPE_SAVEDATA)
            {
                std::u8string savePathU8 = GetSavePath(true).u8string();
                root = (const char *)(savePathU8.c_str());
            }
            else if (pContentData->dwContentType == XCONTENTTYPE_DLC)
            {
                root = GAME_INSTALL_DIRECTORY "/dlc";
            }
            else
            {
                root = GAME_INSTALL_DIRECTORY;
            }

            XamRegisterContent(*pContentData, root);

            std::error_code ec;
            std::filesystem::create_directory(std::u8string_view((const char8_t*)(root.c_str())), ec);

            XamRootCreate(szRootName, root);
        }
        else
        {
            XamRootCreate(szRootName, registry.find(StringHash(pContentData->szFileName))->second.szRoot);
        }

        return ERROR_SUCCESS;
    }

    if (mode == OPEN_EXISTING)
    {
        if (exists)
        {
            if (pdwDisposition)
                *pdwDisposition = XCONTENT_EXISTING;

            XamRootCreate(szRootName, registry.find(StringHash(pContentData->szFileName))->second.szRoot);

            return ERROR_SUCCESS;
        }
        else
        {
            if (pdwDisposition)
                *pdwDisposition = XCONTENT_NEW;

            return ERROR_PATH_NOT_FOUND;
        }
    }

    return ERROR_PATH_NOT_FOUND;
}

uint32_t XamContentClose(const char* szRootName, XXOVERLAPPED* pOverlapped)
{
    gRootMap.erase(StringHash(szRootName));
    return 0;
}

uint32_t XamContentGetDeviceData(uint32_t DeviceID, XDEVICE_DATA* pDeviceData)
{
    pDeviceData->DeviceID = DeviceID;
    pDeviceData->DeviceType = XCONTENTDEVICETYPE_HDD;
    pDeviceData->ulDeviceBytes = 0x10000000;
    pDeviceData->ulDeviceFreeBytes = 0x10000000;
    pDeviceData->wszName[0] = 'S';
    pDeviceData->wszName[1] = 'o';
    pDeviceData->wszName[2] = 'n';
    pDeviceData->wszName[3] = 'i';
    pDeviceData->wszName[4] = 'c';
    pDeviceData->wszName[5] = '\0';

    return 0;
}

uint32_t XamInputGetCapabilities(uint32_t unk, uint32_t userIndex, uint32_t flags, XAMINPUT_CAPABILITIES* caps)
{
    uint32_t result = hid::GetCapabilities(userIndex, caps);

    if (result == ERROR_SUCCESS)
    {
        ByteSwapInplace(caps->Flags);
        ByteSwapInplace(caps->Gamepad.wButtons);
        ByteSwapInplace(caps->Gamepad.sThumbLX);
        ByteSwapInplace(caps->Gamepad.sThumbLY);
        ByteSwapInplace(caps->Gamepad.sThumbRX);
        ByteSwapInplace(caps->Gamepad.sThumbRY);
        ByteSwapInplace(caps->Vibration.wLeftMotorSpeed);
        ByteSwapInplace(caps->Vibration.wRightMotorSpeed);
    }

    return result;
}

uint32_t XamInputGetState(uint32_t userIndex, uint32_t flags, XAMINPUT_STATE* state)
{
    memset(state, 0, sizeof(*state));

    uint32_t result = hid::GetState(userIndex, state);

    if (GameWindow::s_isFocused)
    {
        auto keyboardState = SDL_GetKeyboardState(NULL);

        if (keyboardState[SDL_SCANCODE_UP])
            state->Gamepad.sThumbLY = 32767;
        if (keyboardState[SDL_SCANCODE_DOWN])
            state->Gamepad.sThumbLY = -32768;
        if (keyboardState[SDL_SCANCODE_LEFT])
            state->Gamepad.sThumbLX = -32768;
        if (keyboardState[SDL_SCANCODE_RIGHT])
            state->Gamepad.sThumbLX = 32767;

        if (keyboardState[SDL_SCANCODE_1])
            state->Gamepad.bLeftTrigger = 0xFF;
        if (keyboardState[SDL_SCANCODE_3])
            state->Gamepad.bRightTrigger = 0xFF;

        if (keyboardState[SDL_SCANCODE_I])
            state->Gamepad.wButtons |= XAMINPUT_GAMEPAD_DPAD_UP;
        if (keyboardState[SDL_SCANCODE_K])
            state->Gamepad.wButtons |= XAMINPUT_GAMEPAD_DPAD_DOWN;
        if (keyboardState[SDL_SCANCODE_J])
            state->Gamepad.wButtons |= XAMINPUT_GAMEPAD_DPAD_LEFT;
        if (keyboardState[SDL_SCANCODE_L])
            state->Gamepad.wButtons |= XAMINPUT_GAMEPAD_DPAD_RIGHT;

        if (keyboardState[SDL_SCANCODE_RETURN] && !keyboardState[SDL_SCANCODE_LALT])
            state->Gamepad.wButtons |= XAMINPUT_GAMEPAD_START;
        if (keyboardState[SDL_SCANCODE_BACKSPACE])
            state->Gamepad.wButtons |= XAMINPUT_GAMEPAD_BACK;

        if (keyboardState[SDL_SCANCODE_Q])
            state->Gamepad.wButtons |= XAMINPUT_GAMEPAD_LEFT_SHOULDER;
        if (keyboardState[SDL_SCANCODE_E])
            state->Gamepad.wButtons |= XAMINPUT_GAMEPAD_RIGHT_SHOULDER;

        if (keyboardState[SDL_SCANCODE_S])
            state->Gamepad.wButtons |= XAMINPUT_GAMEPAD_A;
        if (keyboardState[SDL_SCANCODE_D])
            state->Gamepad.wButtons |= XAMINPUT_GAMEPAD_B;
        if (keyboardState[SDL_SCANCODE_A])
            state->Gamepad.wButtons |= XAMINPUT_GAMEPAD_X;
        if (keyboardState[SDL_SCANCODE_W])
            state->Gamepad.wButtons |= XAMINPUT_GAMEPAD_Y;
    }

    state->Gamepad.wButtons &= ~hid::detail::g_prohibitedButtons;

    ByteSwapInplace(state->Gamepad.wButtons);
    ByteSwapInplace(state->Gamepad.sThumbLX);
    ByteSwapInplace(state->Gamepad.sThumbLY);
    ByteSwapInplace(state->Gamepad.sThumbRX);
    ByteSwapInplace(state->Gamepad.sThumbRY);

    return ERROR_SUCCESS;
}

uint32_t XamInputSetState(uint32_t userIndex, uint32_t flags, XAMINPUT_VIBRATION* vibration)
{
    if (!hid::detail::IsInputDeviceController() || !Config::Vibration)
        return ERROR_SUCCESS;

    ByteSwapInplace(vibration->wLeftMotorSpeed);
    ByteSwapInplace(vibration->wRightMotorSpeed);

    return hid::SetState(userIndex, vibration);
}
