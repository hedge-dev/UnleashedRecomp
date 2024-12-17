#pragma once
#include <xbox.h>

#define MSGID(Area, Number)                     (uint32_t)((uint16_t)(Area) << 16 | (uint16_t)(Number))
#define MSG_AREA(msgid)                         (((msgid) >> 16) & 0xFFFF)
#define MSG_NUMBER(msgid)                       ((msgid) & 0xFFFF)

XCONTENT_DATA XamMakeContent(uint32_t type, const std::string_view& name);
void XamRegisterContent(const XCONTENT_DATA& data, const std::string_view& root);

std::string_view XamGetRootPath(const std::string_view& root);
void XamRootCreate(const std::string_view& root, const std::string_view& path);

SWA_API uint32_t XamNotifyCreateListener(uint64_t qwAreas);
SWA_API void XamNotifyEnqueueEvent(uint32_t dwId, uint32_t dwParam); // i made it the fuck up
SWA_API bool XNotifyGetNext(uint32_t hNotification, uint32_t dwMsgFilter, be<uint32_t>* pdwId, be<uint32_t>* pParam);

SWA_API uint32_t XamShowMessageBoxUI(uint32_t dwUserIndex, be<uint16_t>* wszTitle, be<uint16_t>* wszText, uint32_t cButtons,
    xpointer<be<uint16_t>>* pwszButtons, uint32_t dwFocusButton, uint32_t dwFlags, be<uint32_t>* pResult, XXOVERLAPPED* pOverlapped);

SWA_API uint32_t XamContentCreateEnumerator(uint32_t dwUserIndex, uint32_t DeviceID, uint32_t dwContentType,
    uint32_t dwContentFlags, uint32_t cItem, be<uint32_t>* pcbBuffer, be<uint32_t>* phEnum);
SWA_API uint32_t XamEnumerate(uint32_t hEnum, uint32_t dwFlags, void* pvBuffer, uint32_t cbBuffer, be<uint32_t>* pcItemsReturned, XXOVERLAPPED* pOverlapped);

SWA_API uint32_t XamContentCreateEx(uint32_t dwUserIndex, const char* szRootName, const XCONTENT_DATA* pContentData,
    uint32_t dwContentFlags, be<uint32_t>* pdwDisposition, be<uint32_t>* pdwLicenseMask,
    uint32_t dwFileCacheSize, uint64_t uliContentSize, PXXOVERLAPPED pOverlapped);
SWA_API uint32_t XamContentGetDeviceData(uint32_t DeviceID, XDEVICE_DATA* pDeviceData);
SWA_API uint32_t XamContentClose(const char* szRootName, XXOVERLAPPED* pOverlapped);

SWA_API uint32_t XamInputGetCapabilities(uint32_t unk, uint32_t userIndex, uint32_t flags, XAMINPUT_CAPABILITIES* caps);
SWA_API uint32_t XamInputGetState(uint32_t userIndex, uint32_t flags, XAMINPUT_STATE* state);
SWA_API uint32_t XamInputSetState(uint32_t userIndex, uint32_t flags, XAMINPUT_VIBRATION* vibration);
