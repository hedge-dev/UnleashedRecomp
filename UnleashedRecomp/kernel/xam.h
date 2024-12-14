#pragma once
#include <xbox.h>

#define MSGID(Area, Number)                     (DWORD)((WORD)(Area) << 16 | (WORD)(Number))
#define MSG_AREA(msgid)                         (((msgid) >> 16) & 0xFFFF)
#define MSG_NUMBER(msgid)                       ((msgid) & 0xFFFF)

XCONTENT_DATA XamMakeContent(DWORD type, const std::string_view& name);
void XamRegisterContent(const XCONTENT_DATA& data, const std::string_view& root);

std::string_view XamGetRootPath(const std::string_view& root);
void XamRootCreate(const std::string_view& root, const std::string_view& path);

SWA_API DWORD XamNotifyCreateListener(uint64_t qwAreas);
SWA_API void XamNotifyEnqueueEvent(DWORD dwId, DWORD dwParam); // i made it the fuck up
SWA_API bool XNotifyGetNext(DWORD hNotification, DWORD dwMsgFilter, XDWORD* pdwId, XDWORD* pParam);

SWA_API uint32_t XamShowMessageBoxUI(DWORD dwUserIndex, XWORD* wszTitle, XWORD* wszText, DWORD cButtons,
    xpointer<XWORD>* pwszButtons, DWORD dwFocusButton, DWORD dwFlags, XLPDWORD pResult, XXOVERLAPPED* pOverlapped);

SWA_API uint32_t XamContentCreateEnumerator(DWORD dwUserIndex, DWORD DeviceID, DWORD dwContentType,
    DWORD dwContentFlags, DWORD cItem, XLPDWORD pcbBuffer, XLPDWORD phEnum);
SWA_API uint32_t XamEnumerate(uint32_t hEnum, DWORD dwFlags, PVOID pvBuffer, DWORD cbBuffer, XLPDWORD pcItemsReturned, XXOVERLAPPED* pOverlapped);

SWA_API uint32_t XamContentCreateEx(DWORD dwUserIndex, LPCSTR szRootName, const XCONTENT_DATA* pContentData,
    DWORD dwContentFlags, XLPDWORD pdwDisposition, XLPDWORD pdwLicenseMask,
    DWORD dwFileCacheSize, uint64_t uliContentSize, PXXOVERLAPPED pOverlapped);
SWA_API uint32_t XamContentGetDeviceData(DWORD DeviceID, XDEVICE_DATA* pDeviceData);
SWA_API uint32_t XamContentClose(LPCSTR szRootName, XXOVERLAPPED* pOverlapped);

SWA_API uint32_t XamInputGetCapabilities(uint32_t unk, uint32_t userIndex, uint32_t flags, XAMINPUT_CAPABILITIES* caps);
SWA_API uint32_t XamInputGetState(uint32_t userIndex, uint32_t flags, XAMINPUT_STATE* state);
SWA_API uint32_t XamInputSetState(uint32_t userIndex, uint32_t flags, XAMINPUT_VIBRATION* vibration);
