#include <stdafx.h>
#include "xam.h"
#include "xdm.h"
#include <hid/hid.h>
#include <ui/window.h>
#include <cpu/guest_thread.h>
#include <ranges>
#include <unordered_set>
#include <CommCtrl.h>
#include "xxHashMap.h"
#include <user/paths.h>

// Needed for commctrl
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")

std::array<xxHashMap<XHOSTCONTENT_DATA>, 3> gContentRegistry{};
std::unordered_set<XamListener*> gListeners{};
xxHashMap<std::string> gRootMap;

std::string_view XamGetRootPath(const std::string_view& root)
{
    const auto result = gRootMap.find(StringHash(root));
    if (result == gRootMap.end())
    {
        return "";
    }

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

XCONTENT_DATA XamMakeContent(DWORD type, const std::string_view& name)
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

void XamRegisterContent(DWORD type, const std::string_view name, const std::string_view& root)
{
    XCONTENT_DATA data{ 1, type, {}, "" };
    strncpy(data.szFileName, name.data(), sizeof(data.szFileName));

    XamRegisterContent(data, root);
}

SWA_API DWORD XamNotifyCreateListener(uint64_t qwAreas)
{
    int handle;
    auto* listener = ObCreateObject<XamListener>(handle);
    listener->areas = qwAreas;

    return GUEST_HANDLE(handle);
}

SWA_API void XamNotifyEnqueueEvent(DWORD dwId, DWORD dwParam)
{
    for (const auto& listener : gListeners)
    {
        if (((1 << MSG_AREA(dwId)) & listener->areas) == 0)
        {
            continue;
        }

        listener->notifications.emplace_back(dwId, dwParam);
    }
}

SWA_API bool XNotifyGetNext(DWORD hNotification, DWORD dwMsgFilter, XDWORD* pdwId, XDWORD* pParam)
{
    auto& listener = *ObTryQueryObject<XamListener>(HOST_HANDLE(hNotification));
    if (dwMsgFilter)
    {
        for (size_t i = 0; i < listener.notifications.size(); i++)
        {
            if (std::get<0>(listener.notifications[i]) == dwMsgFilter)
            {
                if (pdwId)
                {
                    *pdwId = std::get<0>(listener.notifications[i]);
                }

                if (pParam)
                {
                    *pParam = std::get<1>(listener.notifications[i]);
                }

                listener.notifications.erase(listener.notifications.begin() + i);
                return true;
            }
        }
    }
    else
    {
        if (listener.notifications.empty())
        {
            return false;
        }

        if (pdwId)
        {
            *pdwId = std::get<0>(listener.notifications[0]);
        }

        if (pParam)
        {
            *pParam = std::get<1>(listener.notifications[0]);
        }

        listener.notifications.erase(listener.notifications.begin());
        return true;
    }
    return false;
}

SWA_API uint32_t XamShowMessageBoxUI(DWORD dwUserIndex, XWORD* wszTitle, XWORD* wszText, DWORD cButtons,
    xpointer<XWORD>* pwszButtons, DWORD dwFocusButton, DWORD dwFlags, XLPDWORD pResult, XXOVERLAPPED* pOverlapped)
{
    // printf("!!! STUB !!! XamShowMessageBoxUI\n");

    std::vector<std::wstring> texts{};
    std::vector<TASKDIALOG_BUTTON> buttons{};
    texts.emplace_back(reinterpret_cast<wchar_t*>(wszTitle));
    texts.emplace_back(reinterpret_cast<wchar_t*>(wszText));

    for (size_t i = 0; i < cButtons; i++)
    {
        texts.emplace_back(reinterpret_cast<wchar_t*>(pwszButtons[i].get()));
    }

    for (auto& text : texts)
    {
        for (size_t i = 0; i < text.size(); i++)
        {
            ByteSwap(text[i]);
        }
    }

    for (size_t i = 0; i < cButtons; i++)
    {
        buttons.emplace_back(i, texts[2 + i].c_str());
    }

    XamNotifyEnqueueEvent(9, 1);

    TASKDIALOGCONFIG config{};
    config.cbSize = sizeof(config);
    // config.hwndParent = Window::s_hWnd;
    config.pszWindowTitle = texts[0].c_str();
    config.pszContent = texts[1].c_str();
    config.cButtons = cButtons;
    config.pButtons = buttons.data();

    int button{};
    TaskDialogIndirect(&config, &button, nullptr, nullptr);

    *pResult = button;
    if (pOverlapped)
    {
        pOverlapped->dwCompletionContext = GetCurrentThreadId();
        pOverlapped->Error = 0;
        pOverlapped->Length = -1;
    }

    XamNotifyEnqueueEvent(9, 0);

    return 0;
}

SWA_API uint32_t XamContentCreateEnumerator(DWORD dwUserIndex, DWORD DeviceID, DWORD dwContentType,
    DWORD dwContentFlags, DWORD cItem, XLPDWORD pcbBuffer, XLPDWORD phEnum)
{
    if (dwUserIndex != 0)
    {
        GuestThread::SetLastError(ERROR_NO_SUCH_USER);
        return 0xFFFFFFFF;
    }

    const auto& registry = gContentRegistry[dwContentType - 1];
    const auto& values = registry | std::views::values;
    const int handle = ObInsertObject(new XamEnumerator(cItem, sizeof(_XCONTENT_DATA), values.begin(), values.end()));

    if (pcbBuffer)
    {
        *pcbBuffer = sizeof(_XCONTENT_DATA) * cItem;
    }

    *phEnum = GUEST_HANDLE(handle);
    return 0;
}

SWA_API uint32_t XamEnumerate(uint32_t hEnum, DWORD dwFlags, PVOID pvBuffer, DWORD cbBuffer, XLPDWORD pcItemsReturned, XXOVERLAPPED* pOverlapped)
{
    auto* enumerator = ObTryQueryObject<XamEnumeratorBase>(HOST_HANDLE(hEnum));
    const auto count = enumerator->Next(pvBuffer);
    if (count == -1)
    {
        return ERROR_NO_MORE_FILES;
    }

    if (pcItemsReturned)
    {
        *pcItemsReturned = count;
    }
    return ERROR_SUCCESS;
}

SWA_API uint32_t XamContentCreateEx(DWORD dwUserIndex, LPCSTR szRootName, const XCONTENT_DATA* pContentData,
    DWORD dwContentFlags, XLPDWORD pdwDisposition, XLPDWORD pdwLicenseMask,
    DWORD dwFileCacheSize, uint64_t uliContentSize, PXXOVERLAPPED pOverlapped)
{
    // printf("!!! STUB !!! XamContentCreateEx\n");

    const auto& registry = gContentRegistry[pContentData->dwContentType - 1];
    const auto exists = registry.contains(StringHash(pContentData->szFileName));
    const auto mode = dwContentFlags & 0xF;

    if (mode == CREATE_ALWAYS)
    {
        if (pdwDisposition) *pdwDisposition = XCONTENT_NEW;

        if (!exists)
        {
            std::string root = "";

            if (pContentData->dwContentType == XCONTENTTYPE_SAVEDATA)
            {
                root = GetSavePath().string();
            }
            else if (pContentData->dwContentType == XCONTENTTYPE_DLC)
            {
                root = ".\\dlc";
            }
            else
            {
                root = ".";
            }

            XamRegisterContent(*pContentData, root);
            CreateDirectoryA(root.c_str(), nullptr);
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
            if (pdwDisposition) *pdwDisposition = XCONTENT_EXISTING;

            XamRootCreate(szRootName, registry.find(StringHash(pContentData->szFileName))->second.szRoot);
            return ERROR_SUCCESS;
        }
        else
        {
            if (pdwDisposition) *pdwDisposition = XCONTENT_NEW;
            return ERROR_PATH_NOT_FOUND;
        }
    }

    return ERROR_PATH_NOT_FOUND;
}

SWA_API uint32_t XamContentClose(LPCSTR szRootName, XXOVERLAPPED* pOverlapped)
{
    // printf("!!! STUB !!! XamContentClose %s\n", szRootName);
    gRootMap.erase(StringHash(szRootName));
    return 0;
}

SWA_API uint32_t XamContentGetDeviceData(DWORD DeviceID, XDEVICE_DATA* pDeviceData)
{
    // printf("!!! STUB !!! XamContentGetDeviceData\n");

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

SWA_API uint32_t XamInputGetCapabilities(uint32_t unk, uint32_t userIndex, uint32_t flags, XAMINPUT_CAPABILITIES* caps)
{
    //printf("!!! STUB !!! XamInputGetCapabilities\n");
    uint32_t result = hid::GetCapabilities(userIndex, caps);
    if (result == ERROR_SUCCESS)
    {
        ByteSwap(caps->Flags);
        ByteSwap(caps->Gamepad.wButtons);
        ByteSwap(caps->Gamepad.sThumbLX);
        ByteSwap(caps->Gamepad.sThumbLY);
        ByteSwap(caps->Gamepad.sThumbRX);
        ByteSwap(caps->Gamepad.sThumbRY);
        ByteSwap(caps->Vibration.wLeftMotorSpeed);
        ByteSwap(caps->Vibration.wRightMotorSpeed);
    }
    return result;
}

SWA_API uint32_t XamInputGetState(uint32_t userIndex, uint32_t flags, XAMINPUT_STATE* state)
{
    //printf("!!! STUB !!! XamInputGetState\n");

    uint32_t result = hid::GetState(userIndex, state);

    if (result == ERROR_SUCCESS)
    {
        ByteSwap(state->dwPacketNumber);
        ByteSwap(state->Gamepad.wButtons);
        ByteSwap(state->Gamepad.sThumbLX);
        ByteSwap(state->Gamepad.sThumbLY);
        ByteSwap(state->Gamepad.sThumbRX);
        ByteSwap(state->Gamepad.sThumbRY);
    }
    else if (userIndex == 0)
    {
        if (!Window::s_isFocused)
            return ERROR_SUCCESS;

        memset(state, 0, sizeof(*state));
        if (GetAsyncKeyState('W') & 0x8000)
            state->Gamepad.wButtons |= XAMINPUT_GAMEPAD_Y;
        if (GetAsyncKeyState('A') & 0x8000)
            state->Gamepad.wButtons |= XAMINPUT_GAMEPAD_X;
        if (GetAsyncKeyState('S') & 0x8000)
            state->Gamepad.wButtons |= XAMINPUT_GAMEPAD_A;
        if (GetAsyncKeyState('D') & 0x8000)
            state->Gamepad.wButtons |= XAMINPUT_GAMEPAD_B;
        if (GetAsyncKeyState('Q') & 0x8000)
            state->Gamepad.wButtons |= XAMINPUT_GAMEPAD_LEFT_SHOULDER;
        if (GetAsyncKeyState('E') & 0x8000)
            state->Gamepad.wButtons |= XAMINPUT_GAMEPAD_RIGHT_SHOULDER;
        if (GetAsyncKeyState('1') & 0x8000)
            state->Gamepad.bLeftTrigger = 0xFF;
        if (GetAsyncKeyState('3') & 0x8000)
            state->Gamepad.bRightTrigger = 0xFF;

        if (GetAsyncKeyState('I') & 0x8000)
            state->Gamepad.wButtons |= XAMINPUT_GAMEPAD_DPAD_UP;
        if (GetAsyncKeyState('J') & 0x8000)
            state->Gamepad.wButtons |= XAMINPUT_GAMEPAD_DPAD_LEFT;
        if (GetAsyncKeyState('K') & 0x8000)
            state->Gamepad.wButtons |= XAMINPUT_GAMEPAD_DPAD_DOWN;
        if (GetAsyncKeyState('L') & 0x8000)
            state->Gamepad.wButtons |= XAMINPUT_GAMEPAD_DPAD_RIGHT;

        if (GetAsyncKeyState(VK_UP) & 0x8000)
            state->Gamepad.sThumbLY = 32767;
        if (GetAsyncKeyState(VK_LEFT) & 0x8000)
            state->Gamepad.sThumbLX = -32768;
        if (GetAsyncKeyState(VK_DOWN) & 0x8000)
            state->Gamepad.sThumbLY = -32768;
        if (GetAsyncKeyState(VK_RIGHT) & 0x8000)
            state->Gamepad.sThumbLX = 32767;

        if (GetAsyncKeyState(VK_RETURN) & 0x8000)
            state->Gamepad.wButtons |= XAMINPUT_GAMEPAD_START;

        ByteSwap(state->Gamepad.wButtons);
        ByteSwap(state->Gamepad.sThumbLX);
        ByteSwap(state->Gamepad.sThumbLY);
        ByteSwap(state->Gamepad.sThumbRX);
        ByteSwap(state->Gamepad.sThumbRY);

        result = ERROR_SUCCESS;
    }

    return result;
}

SWA_API uint32_t XamInputSetState(uint32_t userIndex, uint32_t flags, XAMINPUT_VIBRATION* vibration)
{
    //printf("!!! STUB !!! XamInputSetState\n");
    ByteSwap(vibration->wLeftMotorSpeed);
    ByteSwap(vibration->wRightMotorSpeed);
    return hid::SetState(userIndex, vibration);
}
