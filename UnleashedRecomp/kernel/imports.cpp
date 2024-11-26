#include <stdafx.h>
#include <cpu/ppc_context.h>
#include <cpu/guest_thread.h>
#include <apu/audio.h>
#include "function.h"
#include "xex.h"
#include "xbox.h"
#include "heap.h"
#include "memory.h"
#include <memory>
#include "xam.h"
#include "xdm.h"
#include <timeapi.h>
#include <user/config.h>

#include <ntstatus.h>

inline void CloseKernelObject(XDISPATCHER_HEADER& header)
{
    if (header.WaitListHead.Flink != OBJECT_SIGNATURE)
    {
        return;
    }

    ObCloseHandle(header.WaitListHead.Blink);
}

DWORD GuestTimeoutToMilliseconds(XLPQWORD timeout)
{
    return timeout ? (*timeout * -1) / 10000 : INFINITE;
}

void VdHSIOCalibrationLock()
{
    printf("!!! STUB !!! VdHSIOCalibrationLock\n");
}

void KeCertMonitorData()
{
    printf("!!! STUB !!! KeCertMonitorData\n");
}

void XexExecutableModuleHandle()
{
    printf("!!! STUB !!! XexExecutableModuleHandle\n");
}

void ExLoadedCommandLine()
{
    printf("!!! STUB !!! ExLoadedCommandLine\n");
}

void KeDebugMonitorData()
{
    printf("!!! STUB !!! KeDebugMonitorData\n");
}

void ExThreadObjectType()
{
    printf("!!! STUB !!! ExThreadObjectType\n");
}

void KeTimeStampBundle()
{
    printf("!!! STUB !!! KeTimeStampBundle\n");
}

void XboxHardwareInfo()
{
    printf("!!! STUB !!! XboxHardwareInfo\n");
}

void XGetVideoMode()
{
    printf("!!! STUB !!! XGetVideoMode\n");
}

uint32_t XGetGameRegion()
{
    // printf("!!! STUB !!! XGetGameRegion\n");
    if (Config::Language == ELanguage::Japanese)
        return 0x0101;

    return 0x03FF;
}

uint32_t XMsgStartIORequest(DWORD App, DWORD Message, XXOVERLAPPED* lpOverlapped, void* Buffer, DWORD szBuffer)
{
    // printf("!!! STUB !!! XMsgStartIORequest\n");
    return STATUS_SUCCESS;
}

uint32_t XamUserGetSigninState(uint32_t userIndex)
{
    // printf("!!! STUB !!! XamUserGetSigninState\n");
    return userIndex == 0;
}

uint32_t XamGetSystemVersion()
{
    // printf("!!! STUB !!! XamGetSystemVersion\n");
    return 0;
}



void XamContentDelete()
{
    printf("!!! STUB !!! XamContentDelete\n");
}


uint32_t XamContentGetCreator(DWORD userIndex, const XCONTENT_DATA* contentData, PBOOL isCreator, XLPQWORD xuid, XXOVERLAPPED* overlapped)
{
    // printf("!!! STUB !!! XamContentGetCreator\n");

    if (isCreator)
    {
        *isCreator = true;
    }

    if (xuid)
    {
        *xuid = 0xB13EBABEBABEBABE;
    }

    return 0;
}

uint32_t XamContentGetDeviceState()
{
    // printf("!!! STUB !!! XamContentGetDeviceState\n");
    return 0;
}

uint32_t XamUserGetSigninInfo(uint32_t userIndex, uint32_t flags, XUSER_SIGNIN_INFO* info)
{
    //printf("!!! STUB !!! XamUserGetSigninInfo\n");
    if (userIndex == 0)
    {
        memset(info, 0, sizeof(*info));
        info->xuid = 0xB13EBABEBABEBABE;
        info->SigninState = 1;
        strcpy(info->Name, "SWA");
        return 0;
    }

    return 0x00000525;
}

void XamShowSigninUI()
{
    printf("!!! STUB !!! XamShowSigninUI\n");
}

uint32_t XamShowDeviceSelectorUI(
    uint32_t userIndex,
    uint32_t contentType,
    uint32_t contentFlags,
    uint64_t totalRequested,
    XDWORD* deviceId,
    XXOVERLAPPED* overlapped)
{
    // printf("!!! STUB !!! XamShowDeviceSelectorUI\n");
    XamNotifyEnqueueEvent(9, true);
    *deviceId = 1;
    XamNotifyEnqueueEvent(9, false);
    return 0;
}

void XamShowDirtyDiscErrorUI()
{
    printf("!!! STUB !!! XamShowDirtyDiscErrorUI\n");
}

void XamEnableInactivityProcessing()
{
    printf("!!! STUB !!! XamEnableInactivityProcessing\n");
}

void XamResetInactivity()
{
    printf("!!! STUB !!! XamResetInactivity\n");
}

void XamShowMessageBoxUIEx()
{
    printf("!!! STUB !!! XamShowMessageBoxUIEx\n");
}

uint32_t XGetLanguage()
{
    // printf("!!! STUB !!! XGetLanguage\n");
    return (uint32_t)Config::Language.Value;
}

uint32_t XGetAVPack()
{
    // printf("!!! STUB !!! XGetAVPack\n");
    return 0;
}

void XamLoaderTerminateTitle()
{
    printf("!!! STUB !!! XamLoaderTerminateTitle(void)\n");
}

void XamGetExecutionId()
{
    printf("!!! STUB !!! XamGetExecutionId\n");
}

void XamLoaderLaunchTitle()
{
    printf("!!! STUB !!! XamLoaderLaunchTitle(char\n");
}

void NtOpenFile()
{
    printf("!!! STUB !!! NtOpenFile\n");
}

void RtlInitAnsiString(XANSI_STRING* destination, char* source)
{
    printf("!!! STUB !!! RtlInitAnsiString %s\n", source);
    const uint16_t length = source ? (uint16_t)strlen(source) : 0;
    destination->Length = length;
    destination->MaximumLength = length + 1;
    destination->Buffer = source;
}

DWORD NtCreateFile(
    XLPDWORD FileHandle,
    DWORD DesiredAccess,
    XOBJECT_ATTRIBUTES* Attributes,
    XIO_STATUS_BLOCK* IoStatusBlock,
    uint64_t* AllocationSize,
    uint32_t FileAttributes,
    uint32_t ShareAccess,
    uint32_t CreateDisposition,
    uint32_t CreateOptions)
{
    return 0;
}

uint32_t NtClose(uint32_t handle)
{
    if (handle == (uint32_t)INVALID_HANDLE_VALUE)
    {
        return 0xFFFFFFFF;
    }

    if (CHECK_GUEST_HANDLE(handle))
    {
        ObCloseHandle(HOST_HANDLE(handle));
        return 0;
    }

    // printf("!!! STUB !!! NtClose\n");
    return CloseHandle((HANDLE)handle) ? 0 : 0xFFFFFFFF;
}

void NtSetInformationFile()
{
    printf("!!! STUB !!! NtSetInformationFile\n");
}

uint32_t FscSetCacheElementCount()
{
    // printf("!!! STUB !!! FscSetCacheElementCount\n");
    return 0;
}

DWORD NtWaitForSingleObjectEx(DWORD Handle, DWORD WaitMode, DWORD Alertable, XLPQWORD Timeout)
{
    // printf("!!! STUB !!! NtWaitForSingleObjectEx\n");
    const auto status = WaitForSingleObjectEx((HANDLE)Handle, GuestTimeoutToMilliseconds(Timeout), Alertable);

    if (status == WAIT_IO_COMPLETION)
    {
        return STATUS_USER_APC;
    }
    else if (status)
    {
        return STATUS_ALERTED;
    }

    return STATUS_SUCCESS;
}

void NtWriteFile()
{
    printf("!!! STUB !!! NtWriteFile\n");
}

void vsprintf_x()
{
    printf("!!! STUB !!! vsprintf\n");
}

uint32_t ExGetXConfigSetting(uint16_t Category, uint16_t Setting, void* Buffer, uint16_t SizeOfBuffer, XLPDWORD RequiredSize)
{
    // printf("Invoking method ExGetXConfigSetting\n");
    uint32_t data[4]{};

    switch (Category)
    {
        // XCONFIG_SECURED_CATEGORY
    case 0x0002:
    {
        switch (Setting)
        {
            // XCONFIG_SECURED_AV_REGION
        case 0x0002:
            data[0] = std::byteswap(0x00001000); // USA/Canada
            break;

        default:
            return 1;
        }
    }

    case 0x0003:
    {
        switch (Setting)
        {
        case 0x0001: // XCONFIG_USER_TIME_ZONE_BIAS
        case 0x0002: // XCONFIG_USER_TIME_ZONE_STD_NAME
        case 0x0003: // XCONFIG_USER_TIME_ZONE_DLT_NAME
        case 0x0004: // XCONFIG_USER_TIME_ZONE_STD_DATE
        case 0x0005: // XCONFIG_USER_TIME_ZONE_DLT_DATE
        case 0x0006: // XCONFIG_USER_TIME_ZONE_STD_BIAS
        case 0x0007: // XCONFIG_USER_TIME_ZONE_DLT_BIAS
            data[0] = 0;
            break;

            // XCONFIG_USER_LANGUAGE
        case 0x0009:
            data[0] = std::byteswap((uint32_t)Config::Language.Value);
            break;

            // XCONFIG_USER_VIDEO_FLAGS
        case 0x000A:
            data[0] = std::byteswap(0x00040000);
            break;

            // XCONFIG_USER_RETAIL_FLAGS
        case 0x000C:
            data[0] = std::byteswap(1);
            break;

            // XCONFIG_USER_COUNTRY
        case 0x000E:
            data[0] = std::byteswap(103);
            break;

        default:
            return 1;
        }
    }
    }

    *RequiredSize = 4;
    memcpy(Buffer, data, std::min((size_t)SizeOfBuffer, sizeof(data)));

    return 0;
}

void NtQueryVirtualMemory()
{
    printf("!!! STUB !!! NtQueryVirtualMemory\n");
}

void MmQueryStatistics()
{
    printf("!!! STUB !!! MmQueryStatistics\n");
}

uint32_t NtCreateEvent(uint32_t* handle, void* objAttributes, uint32_t eventType, uint32_t initialState)
{
    //printf("!!! STUB !!! NtCreateEvent\n");
    *handle = std::byteswap((uint32_t)CreateEventA(nullptr, !eventType, !!initialState, nullptr));
    return 0;
}

uint32_t XexCheckExecutablePrivilege()
{
    //printf("!!! STUB !!! XexCheckExecutablePrivilege\n");
    return 0;
}

void DbgPrint()
{
    printf("!!! STUB !!! DbgPrint\n");
}

void __C_specific_handler_x()
{
    printf("!!! STUB !!! __C_specific_handler\n");
}

void RtlNtStatusToDosError()
{
    printf("!!! STUB !!! RtlNtStatusToDosError\n");
}

void XexGetProcedureAddress()
{
    printf("!!! STUB !!! XexGetProcedureAddress\n");
}

void XexGetModuleSection()
{
    printf("!!! STUB !!! XexGetModuleSection\n");
}

NTSTATUS RtlUnicodeToMultiByteN(PCHAR MultiByteString, DWORD MaxBytesInMultiByteString, XLPDWORD BytesInMultiByteString, PCWCH UnicodeString, ULONG BytesInUnicodeString)
{
    const auto reqSize = BytesInUnicodeString / sizeof(wchar_t);
    if (BytesInMultiByteString)
    {
        *BytesInMultiByteString = reqSize;
    }

    if (reqSize > MaxBytesInMultiByteString)
    {
        return STATUS_FAIL_CHECK;
    }

    for (size_t i = 0; i < reqSize; i++)
    {
        const auto c = std::byteswap(UnicodeString[i]);
        MultiByteString[i] = c < 256 ? c : '?';
    }

    return STATUS_SUCCESS;
}

DWORD KeDelayExecutionThread(DWORD WaitMode, bool Alertable, XLPQWORD Timeout)
{
    if (Alertable) // We don't do async file reads
        return STATUS_USER_APC;

    // printf("!!! STUB !!! KeDelayExecutionThread\n");

    timeBeginPeriod(1);
    const auto status = SleepEx(GuestTimeoutToMilliseconds(Timeout), Alertable);
    timeEndPeriod(1);

    if (status == WAIT_IO_COMPLETION)
    {
        return STATUS_USER_APC;
    }
    else if (status)
    {
        return STATUS_ALERTED;
    }

    return STATUS_SUCCESS;
}

void ExFreePool()
{
    printf("!!! STUB !!! ExFreePool\n");
}

void NtQueryInformationFile()
{
    printf("!!! STUB !!! NtQueryInformationFile\n");
}

void NtQueryVolumeInformationFile()
{
    printf("!!! STUB !!! NtQueryVolumeInformationFile\n");
}

void NtQueryDirectoryFile()
{
    printf("!!! STUB !!! NtQueryDirectoryFile\n");
}

void NtReadFileScatter()
{
    printf("!!! STUB !!! NtReadFileScatter\n");
}

void NtReadFile()
{
    printf("!!! STUB !!! NtReadFile\n");
}

void NtDuplicateObject()
{
    printf("!!! STUB !!! NtDuplicateObject\n");
}

void NtAllocateVirtualMemory()
{
    printf("!!! STUB !!! NtAllocateVirtualMemory\n");
}

void NtFreeVirtualMemory()
{
    printf("!!! STUB !!! NtFreeVirtualMemory\n");
}

void ObDereferenceObject()
{
    //printf("!!! STUB !!! ObDereferenceObject\n");
}

void KeSetBasePriorityThread(uint32_t thread, int priority)
{
    //printf("!!! STUB !!! KeSetBasePriorityThread\n");
    if (priority == 16)
        priority = 15;
    else if (priority == -16)
        priority = -15;

    SetThreadPriority((HANDLE)thread, priority);
}

uint32_t ObReferenceObjectByHandle(uint32_t handle, uint32_t objectType, XLPDWORD object)
{
    //printf("Invoking method ObReferenceObjectByHandle\n");
    *object = handle;
    return 0;
}

void KeQueryBasePriorityThread()
{
    printf("!!! STUB !!! KeQueryBasePriorityThread\n");
}

uint32_t NtSuspendThread(uint32_t hThread, uint32_t* suspendCount)
{
    //printf("NtSuspendThread(): %x %x\n", hThread, suspendCount);
    DWORD count = SuspendThread((HANDLE)hThread);
    if (count == (DWORD)-1)
        return E_FAIL;

    if (suspendCount != nullptr)
        *suspendCount = std::byteswap(count);

    return S_OK;
}

uint32_t KeSetAffinityThread(DWORD Thread, DWORD Affinity, XLPDWORD lpPreviousAffinity)
{
    // printf("Invoking method KeSetAffinityThread\n");
    if (lpPreviousAffinity)
    {
        *lpPreviousAffinity = 2;
    }
    return 0;
}

struct Event : HostObject<XKEVENT>
{
    HANDLE handle;

    Event(XKEVENT* header)
    {
        handle = CreateEventA(nullptr, !header->Type, !!header->SignalState, nullptr);
    }

    bool Set()
    {
        return SetEvent(handle);
    }

    bool Reset()
    {
        return ResetEvent(handle);
    }
};

struct Semaphore : HostObject<XKSEMAPHORE>
{
    HANDLE handle;

    Semaphore(XKSEMAPHORE* semaphore)
    {
        handle = CreateSemaphoreA(nullptr, semaphore->Header.SignalState, semaphore->Limit, nullptr);
    }
};

// https://devblogs.microsoft.com/oldnewthing/20160825-00/?p=94165

void RtlLeaveCriticalSection(XRTL_CRITICAL_SECTION* cs)
{
    cs->RecursionCount--;
    if (cs->RecursionCount != 0) {
        return;
    }

    InterlockedExchange(&cs->OwningThread, 0);
    WakeByAddressSingle(&cs->OwningThread);
}

void RtlEnterCriticalSection(XRTL_CRITICAL_SECTION* cs)
{
    DWORD thisThread = GetCurrentThreadId();
    while (true) 
    {
        DWORD previousOwner = InterlockedCompareExchangeAcquire(&cs->OwningThread, thisThread, 0);
        if (previousOwner == 0 || previousOwner == thisThread) {
            cs->RecursionCount++;
            return;
        }

        WaitOnAddress(&cs->OwningThread, &previousOwner, sizeof(previousOwner), INFINITE);
    }
}

void RtlImageXexHeaderField()
{
    printf("!!! STUB !!! RtlImageXexHeaderField\n");
}

void HalReturnToFirmware()
{
    printf("!!! STUB !!! HalReturnToFirmware\n");
}

void RtlFillMemoryUlong()
{
    printf("!!! STUB !!! RtlFillMemoryUlong\n");
}

void KeBugCheckEx()
{
    __debugbreak();
}

uint32_t KeGetCurrentProcessType()
{
    //printf("!!! STUB !!! KeGetCurrentProcessType\n");
    return 1;
}

void RtlCompareMemoryUlong()
{
    printf("!!! STUB !!! RtlCompareMemoryUlong\n");
}

uint32_t RtlInitializeCriticalSection(XRTL_CRITICAL_SECTION* cs)
{
    //printf("!!! STUB !!! RtlInitializeCriticalSection\n");
    cs->Header.Absolute = 0;
    cs->LockCount = -1;
    cs->RecursionCount = 0;
    cs->OwningThread = 0;

    return 0;
}

void RtlRaiseException_x()
{
    printf("!!! STUB !!! RtlRaiseException\n");
}

void KfReleaseSpinLock(uint32_t* spinLock)
{
    //printf("!!! STUB !!! KfReleaseSpinLock\n");
    InterlockedExchange((volatile long*)spinLock, 0);
}

void KfAcquireSpinLock(uint32_t* spinLock)
{
    const auto ctx = GetPPCContext();
    //printf("!!! STUB !!! KfAcquireSpinLock\n");

    while (InterlockedCompareExchange((volatile long*)spinLock, std::byteswap(*(uint32_t*)(g_memory.Translate(ctx->r13.u32 + 0x110))), 0) != 0)
    {
        Sleep(0);
    }
}

uint64_t KeQueryPerformanceFrequency()
{
    //printf("!!! STUB !!! KeQueryPerformanceFrequency\n");
    return 49875000;
}

void MmFreePhysicalMemory(uint32_t type, uint32_t guestAddress)
{
    //printf("!!! STUB !!! MmFreePhysicalMemory\n");
    if (guestAddress != NULL)
        g_userHeap.Free(g_memory.Translate(guestAddress));
}

bool VdPersistDisplay(uint32_t a1, uint32_t* a2)
{
    //printf("!!! STUB !!! VdPersistDisplay\n");
    *a2 = NULL;
    return false;
}

void VdSwap()
{
    printf("!!! STUB !!! VdSwap\n");
}

void VdGetSystemCommandBuffer()
{
    printf("!!! STUB !!! VdGetSystemCommandBuffer\n");
}

void KeReleaseSpinLockFromRaisedIrql(uint32_t* spinLock)
{
    //printf("!!! STUB !!! KeReleaseSpinLockFromRaisedIrql\n");
    InterlockedExchange((volatile long*)spinLock, 0);
}

void KeAcquireSpinLockAtRaisedIrql(uint32_t* spinLock)
{
    const auto ctx = GetPPCContext();
    //printf("!!! STUB !!! KeAcquireSpinLockAtRaisedIrql\n");

    while (InterlockedCompareExchange((volatile long*)spinLock, std::byteswap(*(uint32_t*)(g_memory.Translate(ctx->r13.u32 + 0x110))), 0) != 0)
    {
        Sleep(0);
    }
}

uint32_t KiApcNormalRoutineNop()
{
    //printf("Invoking method KiApcNormalRoutineNop\n");
    return 0;
}

void VdEnableRingBufferRPtrWriteBack()
{
    printf("!!! STUB !!! VdEnableRingBufferRPtrWriteBack\n");
}

void VdInitializeRingBuffer()
{
    printf("!!! STUB !!! VdInitializeRingBuffer\n");
}

uint32_t MmGetPhysicalAddress(uint32_t address)
{
    printf("MmGetPhysicalAddress(): %x\n", address);
    return address;
}

void VdSetSystemCommandBufferGpuIdentifierAddress()
{
    printf("!!! STUB !!! VdSetSystemCommandBufferGpuIdentifierAddress\n");
}

void _vsnprintf_x()
{
    printf("!!! STUB !!! _vsnprintf\n");
}

void sprintf_x()
{
    printf("!!! STUB !!! sprintf\n");
}

void ExRegisterTitleTerminateNotification()
{
    printf("!!! STUB !!! ExRegisterTitleTerminateNotification\n");
}

void VdShutdownEngines()
{
    printf("!!! STUB !!! VdShutdownEngines\n");
}

void VdQueryVideoMode(XVIDEO_MODE* vm)
{
    //printf("!!! STUB !!! VdQueryVideoMode\n");
    memset(vm, 0, sizeof(XVIDEO_MODE));
    vm->DisplayWidth = 1280;
    vm->DisplayHeight = 720;
    vm->IsInterlaced = false;
    vm->IsWidescreen = true;
    vm->IsHighDefinition = true;
    vm->RefreshRate = 0x42700000;
    vm->VideoStandard = 1;
    vm->Unknown4A = 0x4A;
    vm->Unknown01 = 0x01;
}

void VdGetCurrentDisplayInformation()
{
    printf("!!! STUB !!! VdGetCurrentDisplayInformation\n");
}

void VdSetDisplayMode()
{
    printf("!!! STUB !!! VdSetDisplayMode\n");
}

void VdSetGraphicsInterruptCallback()
{
    printf("!!! STUB !!! VdSetGraphicsInterruptCallback\n");
}

void VdInitializeEngines()
{
    printf("!!! STUB !!! VdInitializeEngines\n");
}

void VdIsHSIOTrainingSucceeded()
{
    printf("!!! STUB !!! VdIsHSIOTrainingSucceeded\n");
}

void VdGetCurrentDisplayGamma()
{
    printf("!!! STUB !!! VdGetCurrentDisplayGamma\n");
}

void VdQueryVideoFlags()
{
    printf("!!! STUB !!! VdQueryVideoFlags\n");
}

void VdCallGraphicsNotificationRoutines()
{
    printf("!!! STUB !!! VdCallGraphicsNotificationRoutines\n");
}

void VdInitializeScalerCommandBuffer()
{
    printf("!!! STUB !!! VdInitializeScalerCommandBuffer\n");
}

void KeLeaveCriticalRegion()
{
    printf("!!! STUB !!! KeLeaveCriticalRegion\n");
}

uint32_t VdRetrainEDRAM()
{
    //printf("!!! STUB !!! VdRetrainEDRAM\n");
    return 0;
}

void VdRetrainEDRAMWorker()
{
    printf("!!! STUB !!! VdRetrainEDRAMWorker\n");
}

void KeEnterCriticalRegion()
{
    printf("!!! STUB !!! KeEnterCriticalRegion\n");
}

uint32_t MmAllocatePhysicalMemoryEx(
    uint32_t flags,
    uint32_t size,
    uint32_t protect,
    uint32_t minAddress,
    uint32_t maxAddress,
    uint32_t alignment)
{
    printf("MmAllocatePhysicalMemoryEx(): %x %x %x %x %x %x\n", flags, size, protect, minAddress, maxAddress, alignment);
    return g_memory.MapVirtual(g_userHeap.AllocPhysical(size, alignment));
}

void ObDeleteSymbolicLink()
{
    printf("!!! STUB !!! ObDeleteSymbolicLink\n");
}

void ObCreateSymbolicLink()
{
    printf("!!! STUB !!! ObCreateSymbolicLink\n");
}

uint32_t MmQueryAddressProtect(uint32_t guestAddress)
{
    return PAGE_READWRITE;
}

void VdEnableDisableClockGating()
{
    printf("!!! STUB !!! VdEnableDisableClockGating\n");
}

void KeBugCheck()
{
    __debugbreak();
}

void KeLockL2()
{
    printf("!!! STUB !!! KeLockL2\n");
}

void KeUnlockL2()
{
    printf("!!! STUB !!! KeUnlockL2\n");
}

bool KeSetEvent(XKEVENT* pEvent, DWORD Increment, bool Wait)
{
    // printf("!!! STUB !!! KeSetEvent\n");
    return ObQueryObject<Event>(*pEvent)->Set();
}

bool KeResetEvent(XKEVENT* pEvent)
{
    // printf("!!! STUB !!! KeResetEvent %X\n", GetCurrentThreadId());
    return ObQueryObject<Event>(*pEvent)->Reset();
}

DWORD KeWaitForSingleObject(XDISPATCHER_HEADER* Object, DWORD WaitReason, DWORD WaitMode, bool Alertable, XLPQWORD Timeout)
{
    // printf("!!! STUB !!! KeWaitForSingleObject %X\n", GetCurrentThreadId());
    const uint64_t timeout = GuestTimeoutToMilliseconds(Timeout);

    HANDLE handle = nullptr;

    switch (Object->Type)
    {
    case 0:
    case 1:
        handle = ObQueryObject<Event>(*Object)->handle;
        break;

    case 5:
        handle = ObQueryObject<Semaphore>(*Object)->handle;
        break;

    default:
        assert(false);
        break;
    }

    return WaitForSingleObjectEx(handle, timeout, Alertable);
}

uint32_t KeTlsGetValue(DWORD dwTlsIndex)
{
    //printf("!!! STUB !!! KeTlsGetValue\n");
    return (uint32_t)TlsGetValue(dwTlsIndex);
}

BOOL KeTlsSetValue(DWORD dwTlsIndex, DWORD lpTlsValue)
{
    //printf("!!! STUB !!! KeTlsSetValue\n");
    return TlsSetValue(dwTlsIndex, (LPVOID)lpTlsValue);
}

DWORD KeTlsAlloc()
{
    //printf("!!! STUB !!! KeTlsAlloc\n");
    return TlsAlloc();
}

BOOL KeTlsFree(DWORD dwTlsIndex)
{
    //printf("!!! STUB !!! KeTlsFree\n");
    return TlsFree(dwTlsIndex);
}

DWORD XMsgInProcessCall(uint32_t app, uint32_t message, XDWORD* param1, XDWORD* param2)
{
    //printf("!!! STUB !!! XMsgInProcessCall\n");

    if (message == 0x7001B)
    {
        uint32_t* ptr = (uint32_t*)g_memory.Translate(param1[1]);
        ptr[0] = 0;
        ptr[1] = 0;
    }

    return 0;
}

void XamUserReadProfileSettings(
    uint32_t titleId,
    uint32_t userIndex,
    uint32_t xuidCount,
    uint64_t* xuids,
    uint32_t settingCount,
    uint32_t* settingIds,
    XDWORD* bufferSize,
    void* buffer,
    void* overlapped)
{
    //printf("!!! STUB !!! XamUserReadProfileSettings\n");
    if (buffer != nullptr)
    {
        memset(buffer, 0, *bufferSize);
    }
    else
    {
        *bufferSize = 4;
    }
}

void NetDll_WSAStartup()
{
    printf("!!! STUB !!! NetDll_WSAStartup\n");
}

void NetDll_WSACleanup()
{
    printf("!!! STUB !!! NetDll_WSACleanup\n");
}

void NetDll_socket()
{
    printf("!!! STUB !!! NetDll_socket\n");
}

void NetDll_closesocket()
{
    printf("!!! STUB !!! NetDll_closesocket\n");
}

void NetDll_setsockopt()
{
    printf("!!! STUB !!! NetDll_setsockopt\n");
}

void NetDll_bind()
{
    printf("!!! STUB !!! NetDll_bind\n");
}

void NetDll_connect()
{
    printf("!!! STUB !!! NetDll_connect\n");
}

void NetDll_listen()
{
    printf("!!! STUB !!! NetDll_listen\n");
}

void NetDll_accept()
{
    printf("!!! STUB !!! NetDll_accept\n");
}

void NetDll_select()
{
    printf("!!! STUB !!! NetDll_select\n");
}

void NetDll_recv()
{
    printf("!!! STUB !!! NetDll_recv\n");
}

void NetDll_send()
{
    printf("!!! STUB !!! NetDll_send\n");
}

void NetDll_inet_addr()
{
    printf("!!! STUB !!! NetDll_inet_addr\n");
}

void NetDll___WSAFDIsSet()
{
    printf("!!! STUB !!! NetDll___WSAFDIsSet\n");
}

void XMsgStartIORequestEx()
{
    printf("!!! STUB !!! XMsgStartIORequestEx\n");
}

void XexGetModuleHandle()
{
    printf("!!! STUB !!! XexGetModuleHandle\n");
}

bool RtlTryEnterCriticalSection(XRTL_CRITICAL_SECTION* cs)
{
    DWORD thisThread = GetCurrentThreadId();
    DWORD previousOwner = InterlockedCompareExchangeAcquire(&cs->OwningThread, thisThread, 0);

    if (previousOwner == 0 || previousOwner == thisThread) {
        cs->RecursionCount++;
        return true;
    }

    return false;
}

void RtlInitializeCriticalSectionAndSpinCount(XRTL_CRITICAL_SECTION* cs, uint32_t spinCount)
{
    //printf("!!! STUB !!! RtlInitializeCriticalSectionAndSpinCount\n");
    cs->Header.Absolute = (spinCount + 255) >> 8;
    cs->LockCount = -1;
    cs->RecursionCount = 0;
    cs->OwningThread = 0;
}

void _vswprintf_x()
{
    printf("!!! STUB !!! _vswprintf\n");
}

void _vscwprintf_x()
{
    printf("!!! STUB !!! _vscwprintf\n");
}

void _swprintf_x()
{
    printf("!!! STUB !!! _swprintf\n");
}

void _snwprintf_x()
{
    printf("!!! STUB !!! _snwprintf\n");
}

void XeCryptBnQwBeSigVerify()
{
    printf("!!! STUB !!! XeCryptBnQwBeSigVerify\n");
}

void XeKeysGetKey()
{
    printf("!!! STUB !!! XeKeysGetKey\n");
}

void XeCryptRotSumSha()
{
    printf("!!! STUB !!! XeCryptRotSumSha\n");
}

void XeCryptSha()
{
    printf("!!! STUB !!! XeCryptSha\n");
}

void KeEnableFpuExceptions()
{
    printf("!!! STUB !!! KeEnableFpuExceptions\n");
}

void RtlUnwind_x()
{
    printf("!!! STUB !!! RtlUnwind\n");
}

void RtlCaptureContext_x()
{
    printf("!!! STUB !!! RtlCaptureContext\n");
}

void NtQueryFullAttributesFile()
{
    printf("!!! STUB !!! NtQueryFullAttributesFile\n");
}

NTSTATUS RtlMultiByteToUnicodeN(PWCH UnicodeString, ULONG MaxBytesInUnicodeString, XLPDWORD BytesInUnicodeString, const CHAR* MultiByteString, ULONG BytesInMultiByteString)
{
    // i am lazy
    const auto n = MultiByteToWideChar(CP_UTF8, 0, MultiByteString, BytesInMultiByteString, UnicodeString, MaxBytesInUnicodeString);
    if (BytesInUnicodeString)
    {
        *BytesInUnicodeString = n * sizeof(wchar_t);
    }

    if (n)
    {
        for (size_t i = 0; i < n; i++)
        {
            UnicodeString[i] = std::byteswap(UnicodeString[i]);
        }
    }

    return STATUS_SUCCESS;
}

void DbgBreakPoint()
{
    printf("!!! STUB !!! DbgBreakPoint\n");
}

void MmQueryAllocationSize()
{
    printf("!!! STUB !!! MmQueryAllocationSize\n");
}

uint32_t NtClearEvent(uint32_t handle, uint32_t* previousState)
{
    //printf("!!! STUB !!! NtClearEvent\n");
    return ResetEvent((HANDLE)handle) ? 0 : 0xFFFFFFFF;
}

uint32_t NtResumeThread(uint32_t hThread, uint32_t* suspendCount)
{
    //printf("NtResumeThread(): %x %x\n", hThread, suspendCount);
    DWORD count = ResumeThread((HANDLE)hThread);
    if (count == (DWORD)-1)
        return E_FAIL;

    if (suspendCount != nullptr)
        *suspendCount = std::byteswap(count);

    return S_OK;
}

uint32_t NtSetEvent(uint32_t handle, uint32_t* previousState)
{
    //printf("!!! STUB !!! NtSetEvent\n");
    return SetEvent((HANDLE)handle) ? 0 : 0xFFFFFFFF;
}

NTSTATUS NtCreateSemaphore(XLPDWORD Handle, XOBJECT_ATTRIBUTES* ObjectAttributes, DWORD InitialCount, DWORD MaximumCount)
{
    *Handle = (uint32_t)CreateSemaphoreA(nullptr, InitialCount, MaximumCount, nullptr);
    return STATUS_SUCCESS;
}

NTSTATUS NtReleaseSemaphore(uint32_t Handle, DWORD ReleaseCount, LONG* PreviousCount)
{
    //printf("!!! STUB !!! NtReleaseSemaphore\n");
    ReleaseSemaphore((HANDLE)Handle, ReleaseCount, PreviousCount);
    if (PreviousCount)
    {
        *PreviousCount = std::byteswap(*PreviousCount);
    }

    return STATUS_SUCCESS;
}

void NtWaitForMultipleObjectsEx()
{
    printf("!!! STUB !!! NtWaitForMultipleObjectsEx\n");
}

void RtlCompareStringN()
{
    printf("!!! STUB !!! RtlCompareStringN\n");
}

void _snprintf_x()
{
    printf("!!! STUB !!! _snprintf\n");
}

void StfsControlDevice()
{
    printf("!!! STUB !!! StfsControlDevice\n");
}

void StfsCreateDevice()
{
    printf("!!! STUB !!! StfsCreateDevice\n");
}

void NtFlushBuffersFile()
{
    printf("!!! STUB !!! NtFlushBuffersFile\n");
}

void KeQuerySystemTime(uint64_t* time)
{
    //printf("!!! STUB !!! KeQuerySystemTime\n");
    FILETIME t;
    GetSystemTimeAsFileTime(&t);
    *time = std::byteswap((uint64_t(t.dwHighDateTime) << 32) | t.dwLowDateTime);
}

void RtlTimeToTimeFields()
{
    printf("!!! STUB !!! RtlTimeToTimeFields\n");
}

void RtlFreeAnsiString()
{
    printf("!!! STUB !!! RtlFreeAnsiString\n");
}

void RtlUnicodeStringToAnsiString()
{
    printf("!!! STUB !!! RtlUnicodeStringToAnsiString\n");
}

void RtlInitUnicodeString()
{
    printf("!!! STUB !!! RtlInitUnicodeString\n");
}

void ExTerminateThread()
{
    printf("!!! STUB !!! ExTerminateThread\n");
}

uint32_t ExCreateThread(XLPDWORD handle, uint32_t stackSize, XLPDWORD threadId, uint32_t xApiThreadStartup, uint32_t startAddress, uint32_t startContext, uint32_t creationFlags)
{
    printf("ExCreateThread(): %p %x %p %x %x %x %x\n", handle, stackSize, threadId, xApiThreadStartup, startAddress, startContext, creationFlags);
    DWORD hostThreadId;

    *handle = (uint32_t)GuestThread::Start(startAddress, startContext, creationFlags, &hostThreadId);
    if (threadId != nullptr)
        *threadId = hostThreadId;

    return 0;
}

void IoInvalidDeviceRequest()
{
    printf("!!! STUB !!! IoInvalidDeviceRequest\n");
}

void ObReferenceObject()
{
    printf("!!! STUB !!! ObReferenceObject\n");
}

void IoCreateDevice()
{
    printf("!!! STUB !!! IoCreateDevice\n");
}

void IoDeleteDevice()
{
    printf("!!! STUB !!! IoDeleteDevice\n");
}

void ExAllocatePoolTypeWithTag()
{
    printf("!!! STUB !!! ExAllocatePoolTypeWithTag\n");
}

void RtlTimeFieldsToTime()
{
    printf("!!! STUB !!! RtlTimeFieldsToTime\n");
}

void IoCompleteRequest()
{
    printf("!!! STUB !!! IoCompleteRequest\n");
}

void RtlUpcaseUnicodeChar()
{
    printf("!!! STUB !!! RtlUpcaseUnicodeChar\n");
}

void ObIsTitleObject()
{
    printf("!!! STUB !!! ObIsTitleObject\n");
}

void IoCheckShareAccess()
{
    printf("!!! STUB !!! IoCheckShareAccess\n");
}

void IoSetShareAccess()
{
    printf("!!! STUB !!! IoSetShareAccess\n");
}

void IoRemoveShareAccess()
{
    printf("!!! STUB !!! IoRemoveShareAccess\n");
}

void NetDll_XNetStartup()
{
    printf("!!! STUB !!! NetDll_XNetStartup\n");
}

void NetDll_XNetGetTitleXnAddr()
{
    printf("!!! STUB !!! NetDll_XNetGetTitleXnAddr\n");
}

DWORD KeWaitForMultipleObjects(DWORD Count, xpointer<XDISPATCHER_HEADER>* Objects, DWORD WaitType, DWORD WaitReason, DWORD WaitMode, DWORD Alertable, XLPQWORD Timeout)
{
    // TODO: Create actual objects by type
    const uint64_t timeout = GuestTimeoutToMilliseconds(Timeout);

    thread_local std::vector<HANDLE> events;
    events.resize(Count);

    for (size_t i = 0; i < Count; i++)
    {
        assert(Objects[i]->Type <= 1);
        events[i] = ObQueryObject<Event>(*Objects[i].get())->handle;
    }

    return WaitForMultipleObjectsEx(Count, events.data(), WaitType == 0, timeout, Alertable);
}

uint32_t KeRaiseIrqlToDpcLevel()
{
    //printf("!!! STUB !!! KeRaiseIrqlToDpcLevel\n");
    return 0;
}

void KfLowerIrql()
{
    //printf("!!! STUB !!! KfLowerIrql\n");
}

uint32_t KeReleaseSemaphore(XKSEMAPHORE* semaphore, uint32_t increment, uint32_t adjustment, uint32_t wait)
{
    //printf("!!! STUB !!! KeReleaseSemaphore\n");
    auto* object = ObQueryObject<Semaphore>(semaphore->Header);
    return ReleaseSemaphore(object->handle, adjustment, nullptr) ? 0 : 0xFFFFFFFF;
}

void XAudioGetVoiceCategoryVolume()
{
    printf("!!! STUB !!! XAudioGetVoiceCategoryVolume\n");
}

DWORD XAudioGetVoiceCategoryVolumeChangeMask(DWORD Driver, XLPDWORD Mask)
{
    // printf("Invoking method XAudioGetVoiceCategoryVolumeChangeMask\n");
    *Mask = 0;

    return 0;
}

uint32_t KeResumeThread(uint32_t object)
{
    printf("KeResumeThread(): %x\n", object);
    return ResumeThread((HANDLE)object);
}

void KeInitializeSemaphore(XKSEMAPHORE* semaphore, uint32_t count, uint32_t limit)
{
    //printf("!!! STUB !!! KeInitializeSemaphore\n");
    semaphore->Header.Type = 5;
    semaphore->Header.SignalState = count;
    semaphore->Limit = limit;
    auto* object = ObQueryObject<Semaphore>(semaphore->Header);
}

void XMAReleaseContext()
{
    printf("!!! STUB !!! XMAReleaseContext\n");
}

void XMACreateContext()
{
    printf("!!! STUB !!! XMACreateContext\n");
}

//uint32_t XAudioRegisterRenderDriverClient(XLPDWORD callback, XLPDWORD driver)
//{
//    //printf("XAudioRegisterRenderDriverClient(): %x %x\n");
//
//    *driver = apu::RegisterClient(callback[0], callback[1]);
//    return 0;
//}
//
//void XAudioUnregisterRenderDriverClient()
//{
//    printf("!!! STUB !!! XAudioUnregisterRenderDriverClient\n");
//}
//
//uint32_t XAudioSubmitRenderDriverFrame(uint32_t driver, void* samples)
//{
//    // printf("!!! STUB !!! XAudioSubmitRenderDriverFrame\n");
//    apu::SubmitFrames(samples);
//
//    return 0;
//}

GUEST_FUNCTION_HOOK(__imp__XGetVideoMode, VdQueryVideoMode); // XGetVideoMode
GUEST_FUNCTION_HOOK(__imp__XNotifyGetNext, XNotifyGetNext);
GUEST_FUNCTION_HOOK(__imp__XGetGameRegion, XGetGameRegion);
GUEST_FUNCTION_HOOK(__imp__XMsgStartIORequest, XMsgStartIORequest);
GUEST_FUNCTION_HOOK(__imp__XamUserGetSigninState, XamUserGetSigninState);
GUEST_FUNCTION_HOOK(__imp__XamGetSystemVersion, XamGetSystemVersion);
GUEST_FUNCTION_HOOK(__imp__XamContentCreateEx, XamContentCreateEx);
GUEST_FUNCTION_HOOK(__imp__XamContentDelete, XamContentDelete);
GUEST_FUNCTION_HOOK(__imp__XamContentClose, XamContentClose);
GUEST_FUNCTION_HOOK(__imp__XamContentGetCreator, XamContentGetCreator);
GUEST_FUNCTION_HOOK(__imp__XamContentCreateEnumerator, XamContentCreateEnumerator);
GUEST_FUNCTION_HOOK(__imp__XamContentGetDeviceState, XamContentGetDeviceState);
GUEST_FUNCTION_HOOK(__imp__XamContentGetDeviceData, XamContentGetDeviceData);
GUEST_FUNCTION_HOOK(__imp__XamEnumerate, XamEnumerate);
GUEST_FUNCTION_HOOK(__imp__XamNotifyCreateListener, XamNotifyCreateListener);
GUEST_FUNCTION_HOOK(__imp__XamUserGetSigninInfo, XamUserGetSigninInfo);
GUEST_FUNCTION_HOOK(__imp__XamShowSigninUI, XamShowSigninUI);
GUEST_FUNCTION_HOOK(__imp__XamShowDeviceSelectorUI, XamShowDeviceSelectorUI);
GUEST_FUNCTION_HOOK(__imp__XamShowMessageBoxUI, XamShowMessageBoxUI);
GUEST_FUNCTION_HOOK(__imp__XamShowDirtyDiscErrorUI, XamShowDirtyDiscErrorUI);
GUEST_FUNCTION_HOOK(__imp__XamEnableInactivityProcessing, XamEnableInactivityProcessing);
GUEST_FUNCTION_HOOK(__imp__XamResetInactivity, XamResetInactivity);
GUEST_FUNCTION_HOOK(__imp__XamShowMessageBoxUIEx, XamShowMessageBoxUIEx);
GUEST_FUNCTION_HOOK(__imp__XGetLanguage, XGetLanguage);
GUEST_FUNCTION_HOOK(__imp__XGetAVPack, XGetAVPack);
GUEST_FUNCTION_HOOK(__imp__XamLoaderTerminateTitle, XamLoaderTerminateTitle);
GUEST_FUNCTION_HOOK(__imp__XamGetExecutionId, XamGetExecutionId);
GUEST_FUNCTION_HOOK(__imp__XamLoaderLaunchTitle, XamLoaderLaunchTitle);
GUEST_FUNCTION_HOOK(__imp__NtOpenFile, NtOpenFile);
GUEST_FUNCTION_HOOK(__imp__RtlInitAnsiString, RtlInitAnsiString);
GUEST_FUNCTION_HOOK(__imp__NtCreateFile, NtCreateFile);
GUEST_FUNCTION_HOOK(__imp__NtClose, NtClose);
GUEST_FUNCTION_HOOK(__imp__NtSetInformationFile, NtSetInformationFile);
GUEST_FUNCTION_HOOK(__imp__FscSetCacheElementCount, FscSetCacheElementCount);
GUEST_FUNCTION_HOOK(__imp__NtWaitForSingleObjectEx, NtWaitForSingleObjectEx);
GUEST_FUNCTION_HOOK(__imp__NtWriteFile, NtWriteFile);
GUEST_FUNCTION_HOOK(__imp__ExGetXConfigSetting, ExGetXConfigSetting);
GUEST_FUNCTION_HOOK(__imp__NtQueryVirtualMemory, NtQueryVirtualMemory);
GUEST_FUNCTION_HOOK(__imp__MmQueryStatistics, MmQueryStatistics);
GUEST_FUNCTION_HOOK(__imp__NtCreateEvent, NtCreateEvent);
GUEST_FUNCTION_HOOK(__imp__XexCheckExecutablePrivilege, XexCheckExecutablePrivilege);
GUEST_FUNCTION_HOOK(__imp__DbgPrint, DbgPrint);
GUEST_FUNCTION_HOOK(__imp____C_specific_handler, __C_specific_handler_x);
GUEST_FUNCTION_HOOK(__imp__RtlNtStatusToDosError, RtlNtStatusToDosError);
GUEST_FUNCTION_HOOK(__imp__XexGetProcedureAddress, XexGetProcedureAddress);
GUEST_FUNCTION_HOOK(__imp__XexGetModuleSection, XexGetModuleSection);
GUEST_FUNCTION_HOOK(__imp__RtlUnicodeToMultiByteN, RtlUnicodeToMultiByteN);
GUEST_FUNCTION_HOOK(__imp__KeDelayExecutionThread, KeDelayExecutionThread);
GUEST_FUNCTION_HOOK(__imp__ExFreePool, ExFreePool);
GUEST_FUNCTION_HOOK(__imp__NtQueryInformationFile, NtQueryInformationFile);
GUEST_FUNCTION_HOOK(__imp__NtQueryVolumeInformationFile, NtQueryVolumeInformationFile);
GUEST_FUNCTION_HOOK(__imp__NtQueryDirectoryFile, NtQueryDirectoryFile);
GUEST_FUNCTION_HOOK(__imp__NtReadFileScatter, NtReadFileScatter);
GUEST_FUNCTION_HOOK(__imp__NtReadFile, NtReadFile);
GUEST_FUNCTION_HOOK(__imp__NtDuplicateObject, NtDuplicateObject);
GUEST_FUNCTION_HOOK(__imp__NtAllocateVirtualMemory, NtAllocateVirtualMemory);
GUEST_FUNCTION_HOOK(__imp__NtFreeVirtualMemory, NtFreeVirtualMemory);
GUEST_FUNCTION_HOOK(__imp__ObDereferenceObject, ObDereferenceObject);
GUEST_FUNCTION_HOOK(__imp__KeSetBasePriorityThread, KeSetBasePriorityThread);
GUEST_FUNCTION_HOOK(__imp__ObReferenceObjectByHandle, ObReferenceObjectByHandle);
GUEST_FUNCTION_HOOK(__imp__KeQueryBasePriorityThread, KeQueryBasePriorityThread);
GUEST_FUNCTION_HOOK(__imp__NtSuspendThread, NtSuspendThread);
GUEST_FUNCTION_HOOK(__imp__KeSetAffinityThread, KeSetAffinityThread);
GUEST_FUNCTION_HOOK(__imp__RtlLeaveCriticalSection, RtlLeaveCriticalSection);
GUEST_FUNCTION_HOOK(__imp__RtlEnterCriticalSection, RtlEnterCriticalSection);
GUEST_FUNCTION_HOOK(__imp__RtlImageXexHeaderField, RtlImageXexHeaderField);
GUEST_FUNCTION_HOOK(__imp__HalReturnToFirmware, HalReturnToFirmware);
GUEST_FUNCTION_HOOK(__imp__RtlFillMemoryUlong, RtlFillMemoryUlong);
GUEST_FUNCTION_HOOK(__imp__KeBugCheckEx, KeBugCheckEx);
GUEST_FUNCTION_HOOK(__imp__KeGetCurrentProcessType, KeGetCurrentProcessType);
GUEST_FUNCTION_HOOK(__imp__RtlCompareMemoryUlong, RtlCompareMemoryUlong);
GUEST_FUNCTION_HOOK(__imp__RtlInitializeCriticalSection, RtlInitializeCriticalSection);
GUEST_FUNCTION_HOOK(__imp__RtlRaiseException, RtlRaiseException_x);
GUEST_FUNCTION_HOOK(__imp__KfReleaseSpinLock, KfReleaseSpinLock);
GUEST_FUNCTION_HOOK(__imp__KfAcquireSpinLock, KfAcquireSpinLock);
GUEST_FUNCTION_HOOK(__imp__KeQueryPerformanceFrequency, KeQueryPerformanceFrequency);
GUEST_FUNCTION_HOOK(__imp__MmFreePhysicalMemory, MmFreePhysicalMemory);
GUEST_FUNCTION_HOOK(__imp__VdPersistDisplay, VdPersistDisplay);
GUEST_FUNCTION_HOOK(__imp__VdSwap, VdSwap);
GUEST_FUNCTION_HOOK(__imp__VdGetSystemCommandBuffer, VdGetSystemCommandBuffer);
GUEST_FUNCTION_HOOK(__imp__KeReleaseSpinLockFromRaisedIrql, KeReleaseSpinLockFromRaisedIrql);
GUEST_FUNCTION_HOOK(__imp__KeAcquireSpinLockAtRaisedIrql, KeAcquireSpinLockAtRaisedIrql);
GUEST_FUNCTION_HOOK(__imp__KiApcNormalRoutineNop, KiApcNormalRoutineNop);
GUEST_FUNCTION_HOOK(__imp__VdEnableRingBufferRPtrWriteBack, VdEnableRingBufferRPtrWriteBack);
GUEST_FUNCTION_HOOK(__imp__VdInitializeRingBuffer, VdInitializeRingBuffer);
GUEST_FUNCTION_HOOK(__imp__MmGetPhysicalAddress, MmGetPhysicalAddress);
GUEST_FUNCTION_HOOK(__imp__VdSetSystemCommandBufferGpuIdentifierAddress, VdSetSystemCommandBufferGpuIdentifierAddress);
GUEST_FUNCTION_HOOK(__imp__ExRegisterTitleTerminateNotification, ExRegisterTitleTerminateNotification);
GUEST_FUNCTION_HOOK(__imp__VdShutdownEngines, VdShutdownEngines);
GUEST_FUNCTION_HOOK(__imp__VdQueryVideoMode, VdQueryVideoMode);
GUEST_FUNCTION_HOOK(__imp__VdGetCurrentDisplayInformation, VdGetCurrentDisplayInformation);
GUEST_FUNCTION_HOOK(__imp__VdSetDisplayMode, VdSetDisplayMode);
GUEST_FUNCTION_HOOK(__imp__VdSetGraphicsInterruptCallback, VdSetGraphicsInterruptCallback);
GUEST_FUNCTION_HOOK(__imp__VdInitializeEngines, VdInitializeEngines);
GUEST_FUNCTION_HOOK(__imp__VdIsHSIOTrainingSucceeded, VdIsHSIOTrainingSucceeded);
GUEST_FUNCTION_HOOK(__imp__VdGetCurrentDisplayGamma, VdGetCurrentDisplayGamma);
GUEST_FUNCTION_HOOK(__imp__VdQueryVideoFlags, VdQueryVideoFlags);
GUEST_FUNCTION_HOOK(__imp__VdCallGraphicsNotificationRoutines, VdCallGraphicsNotificationRoutines);
GUEST_FUNCTION_HOOK(__imp__VdInitializeScalerCommandBuffer, VdInitializeScalerCommandBuffer);
GUEST_FUNCTION_HOOK(__imp__KeLeaveCriticalRegion, KeLeaveCriticalRegion);
GUEST_FUNCTION_HOOK(__imp__VdRetrainEDRAM, VdRetrainEDRAM);
GUEST_FUNCTION_HOOK(__imp__VdRetrainEDRAMWorker, VdRetrainEDRAMWorker);
GUEST_FUNCTION_HOOK(__imp__KeEnterCriticalRegion, KeEnterCriticalRegion);
GUEST_FUNCTION_HOOK(__imp__MmAllocatePhysicalMemoryEx, MmAllocatePhysicalMemoryEx);
GUEST_FUNCTION_HOOK(__imp__ObDeleteSymbolicLink, ObDeleteSymbolicLink);
GUEST_FUNCTION_HOOK(__imp__ObCreateSymbolicLink, ObCreateSymbolicLink);
GUEST_FUNCTION_HOOK(__imp__MmQueryAddressProtect, MmQueryAddressProtect);
GUEST_FUNCTION_HOOK(__imp__VdEnableDisableClockGating, VdEnableDisableClockGating);
GUEST_FUNCTION_HOOK(__imp__KeBugCheck, KeBugCheck);
GUEST_FUNCTION_HOOK(__imp__KeLockL2, KeLockL2);
GUEST_FUNCTION_HOOK(__imp__KeUnlockL2, KeUnlockL2);
GUEST_FUNCTION_HOOK(__imp__KeSetEvent, KeSetEvent);
GUEST_FUNCTION_HOOK(__imp__KeResetEvent, KeResetEvent);
GUEST_FUNCTION_HOOK(__imp__KeWaitForSingleObject, KeWaitForSingleObject);
GUEST_FUNCTION_HOOK(__imp__KeTlsGetValue, KeTlsGetValue);
GUEST_FUNCTION_HOOK(__imp__KeTlsSetValue, KeTlsSetValue);
GUEST_FUNCTION_HOOK(__imp__KeTlsAlloc, KeTlsAlloc);
GUEST_FUNCTION_HOOK(__imp__KeTlsFree, KeTlsFree);
GUEST_FUNCTION_HOOK(__imp__XMsgInProcessCall, XMsgInProcessCall);
GUEST_FUNCTION_HOOK(__imp__XamUserReadProfileSettings, XamUserReadProfileSettings);
GUEST_FUNCTION_HOOK(__imp__NetDll_WSAStartup, NetDll_WSAStartup);
GUEST_FUNCTION_HOOK(__imp__NetDll_WSACleanup, NetDll_WSACleanup);
GUEST_FUNCTION_HOOK(__imp__NetDll_socket, NetDll_socket);
GUEST_FUNCTION_HOOK(__imp__NetDll_closesocket, NetDll_closesocket);
GUEST_FUNCTION_HOOK(__imp__NetDll_setsockopt, NetDll_setsockopt);
GUEST_FUNCTION_HOOK(__imp__NetDll_bind, NetDll_bind);
GUEST_FUNCTION_HOOK(__imp__NetDll_connect, NetDll_connect);
GUEST_FUNCTION_HOOK(__imp__NetDll_listen, NetDll_listen);
GUEST_FUNCTION_HOOK(__imp__NetDll_accept, NetDll_accept);
GUEST_FUNCTION_HOOK(__imp__NetDll_select, NetDll_select);
GUEST_FUNCTION_HOOK(__imp__NetDll_recv, NetDll_recv);
GUEST_FUNCTION_HOOK(__imp__NetDll_send, NetDll_send);
GUEST_FUNCTION_HOOK(__imp__NetDll_inet_addr, NetDll_inet_addr);
GUEST_FUNCTION_HOOK(__imp__NetDll___WSAFDIsSet, NetDll___WSAFDIsSet);
GUEST_FUNCTION_HOOK(__imp__XMsgStartIORequestEx, XMsgStartIORequestEx);
GUEST_FUNCTION_HOOK(__imp__XamInputGetCapabilities, XamInputGetCapabilities);
GUEST_FUNCTION_HOOK(__imp__XamInputGetState, XamInputGetState);
GUEST_FUNCTION_HOOK(__imp__XamInputSetState, XamInputSetState);
GUEST_FUNCTION_HOOK(__imp__XexGetModuleHandle, XexGetModuleHandle);
GUEST_FUNCTION_HOOK(__imp__RtlTryEnterCriticalSection, RtlTryEnterCriticalSection);
GUEST_FUNCTION_HOOK(__imp__RtlInitializeCriticalSectionAndSpinCount, RtlInitializeCriticalSectionAndSpinCount);
GUEST_FUNCTION_HOOK(__imp__XeCryptBnQwBeSigVerify, XeCryptBnQwBeSigVerify);
GUEST_FUNCTION_HOOK(__imp__XeKeysGetKey, XeKeysGetKey);
GUEST_FUNCTION_HOOK(__imp__XeCryptRotSumSha, XeCryptRotSumSha);
GUEST_FUNCTION_HOOK(__imp__XeCryptSha, XeCryptSha);
GUEST_FUNCTION_HOOK(__imp__KeEnableFpuExceptions, KeEnableFpuExceptions);
GUEST_FUNCTION_HOOK(__imp__RtlUnwind, RtlUnwind_x);
GUEST_FUNCTION_HOOK(__imp__RtlCaptureContext, RtlCaptureContext_x);
GUEST_FUNCTION_HOOK(__imp__NtQueryFullAttributesFile, NtQueryFullAttributesFile);
GUEST_FUNCTION_HOOK(__imp__RtlMultiByteToUnicodeN, RtlMultiByteToUnicodeN);
GUEST_FUNCTION_HOOK(__imp__DbgBreakPoint, DbgBreakPoint);
GUEST_FUNCTION_HOOK(__imp__MmQueryAllocationSize, MmQueryAllocationSize);
GUEST_FUNCTION_HOOK(__imp__NtClearEvent, NtClearEvent);
GUEST_FUNCTION_HOOK(__imp__NtResumeThread, NtResumeThread);
GUEST_FUNCTION_HOOK(__imp__NtSetEvent, NtSetEvent);
GUEST_FUNCTION_HOOK(__imp__NtCreateSemaphore, NtCreateSemaphore);
GUEST_FUNCTION_HOOK(__imp__NtReleaseSemaphore, NtReleaseSemaphore);
GUEST_FUNCTION_HOOK(__imp__NtWaitForMultipleObjectsEx, NtWaitForMultipleObjectsEx);
GUEST_FUNCTION_HOOK(__imp__RtlCompareStringN, RtlCompareStringN);
GUEST_FUNCTION_HOOK(__imp__StfsControlDevice, StfsControlDevice);
GUEST_FUNCTION_HOOK(__imp__StfsCreateDevice, StfsCreateDevice);
GUEST_FUNCTION_HOOK(__imp__NtFlushBuffersFile, NtFlushBuffersFile);
GUEST_FUNCTION_HOOK(__imp__KeQuerySystemTime, KeQuerySystemTime);
GUEST_FUNCTION_HOOK(__imp__RtlTimeToTimeFields, RtlTimeToTimeFields);
GUEST_FUNCTION_HOOK(__imp__RtlFreeAnsiString, RtlFreeAnsiString);
GUEST_FUNCTION_HOOK(__imp__RtlUnicodeStringToAnsiString, RtlUnicodeStringToAnsiString);
GUEST_FUNCTION_HOOK(__imp__RtlInitUnicodeString, RtlInitUnicodeString);
GUEST_FUNCTION_HOOK(__imp__ExTerminateThread, ExTerminateThread);
GUEST_FUNCTION_HOOK(__imp__ExCreateThread, ExCreateThread);
GUEST_FUNCTION_HOOK(__imp__IoInvalidDeviceRequest, IoInvalidDeviceRequest);
GUEST_FUNCTION_HOOK(__imp__ObReferenceObject, ObReferenceObject);
GUEST_FUNCTION_HOOK(__imp__IoCreateDevice, IoCreateDevice);
GUEST_FUNCTION_HOOK(__imp__IoDeleteDevice, IoDeleteDevice);
GUEST_FUNCTION_HOOK(__imp__ExAllocatePoolTypeWithTag, ExAllocatePoolTypeWithTag);
GUEST_FUNCTION_HOOK(__imp__RtlTimeFieldsToTime, RtlTimeFieldsToTime);
GUEST_FUNCTION_HOOK(__imp__IoCompleteRequest, IoCompleteRequest);
GUEST_FUNCTION_HOOK(__imp__RtlUpcaseUnicodeChar, RtlUpcaseUnicodeChar);
GUEST_FUNCTION_HOOK(__imp__ObIsTitleObject, ObIsTitleObject);
GUEST_FUNCTION_HOOK(__imp__IoCheckShareAccess, IoCheckShareAccess);
GUEST_FUNCTION_HOOK(__imp__IoSetShareAccess, IoSetShareAccess);
GUEST_FUNCTION_HOOK(__imp__IoRemoveShareAccess, IoRemoveShareAccess);
GUEST_FUNCTION_HOOK(__imp__NetDll_XNetStartup, NetDll_XNetStartup);
GUEST_FUNCTION_HOOK(__imp__NetDll_XNetGetTitleXnAddr, NetDll_XNetGetTitleXnAddr);
GUEST_FUNCTION_HOOK(__imp__KeWaitForMultipleObjects, KeWaitForMultipleObjects);
GUEST_FUNCTION_HOOK(__imp__KeRaiseIrqlToDpcLevel, KeRaiseIrqlToDpcLevel);
GUEST_FUNCTION_HOOK(__imp__KfLowerIrql, KfLowerIrql);
GUEST_FUNCTION_HOOK(__imp__KeReleaseSemaphore, KeReleaseSemaphore);
GUEST_FUNCTION_HOOK(__imp__XAudioGetVoiceCategoryVolume, XAudioGetVoiceCategoryVolume);
GUEST_FUNCTION_HOOK(__imp__XAudioGetVoiceCategoryVolumeChangeMask, XAudioGetVoiceCategoryVolumeChangeMask);
GUEST_FUNCTION_HOOK(__imp__KeResumeThread, KeResumeThread);
GUEST_FUNCTION_HOOK(__imp__KeInitializeSemaphore, KeInitializeSemaphore);
GUEST_FUNCTION_HOOK(__imp__XMAReleaseContext, XMAReleaseContext);
GUEST_FUNCTION_HOOK(__imp__XMACreateContext, XMACreateContext);
GUEST_FUNCTION_HOOK(__imp__XAudioRegisterRenderDriverClient, XAudioRegisterRenderDriverClient);
GUEST_FUNCTION_HOOK(__imp__XAudioUnregisterRenderDriverClient, XAudioUnregisterRenderDriverClient);
GUEST_FUNCTION_HOOK(__imp__XAudioSubmitRenderDriverFrame, XAudioSubmitRenderDriverFrame);
