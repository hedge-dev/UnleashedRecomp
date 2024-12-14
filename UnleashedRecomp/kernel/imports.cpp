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
#include <os/logger.h>

#include <ntstatus.h>

inline void CloseKernelObject(XDISPATCHER_HEADER& header)
{
    if (header.WaitListHead.Flink != OBJECT_SIGNATURE)
    {
        return;
    }

    DestroyKernelObject(header.WaitListHead.Blink);
}

DWORD GuestTimeoutToMilliseconds(XLPQWORD timeout)
{
    return timeout ? (*timeout * -1) / 10000 : INFINITE;
}

void VdHSIOCalibrationLock()
{
    LOG_UTILITY("!!! STUB !!!");
}

void KeCertMonitorData()
{
    LOG_UTILITY("!!! STUB !!!");
}

void XexExecutableModuleHandle()
{
    LOG_UTILITY("!!! STUB !!!");
}

void ExLoadedCommandLine()
{
    LOG_UTILITY("!!! STUB !!!");
}

void KeDebugMonitorData()
{
    LOG_UTILITY("!!! STUB !!!");
}

void ExThreadObjectType()
{
    LOG_UTILITY("!!! STUB !!!");
}

void KeTimeStampBundle()
{
    LOG_UTILITY("!!! STUB !!!");
}

void XboxHardwareInfo()
{
    LOG_UTILITY("!!! STUB !!!");
}

void XGetVideoMode()
{
    LOG_UTILITY("!!! STUB !!!");
}

uint32_t XGetGameRegion()
{
    if (Config::Language == ELanguage::Japanese)
        return 0x0101;

    return 0x03FF;
}

uint32_t XMsgStartIORequest(DWORD App, DWORD Message, XXOVERLAPPED* lpOverlapped, void* Buffer, DWORD szBuffer)
{
    return STATUS_SUCCESS;
}

uint32_t XamUserGetSigninState(uint32_t userIndex)
{
    return true;
}

uint32_t XamGetSystemVersion()
{
    return 0;
}

void XamContentDelete()
{
    LOG_UTILITY("!!! STUB !!!");
}


uint32_t XamContentGetCreator(DWORD userIndex, const XCONTENT_DATA* contentData, PBOOL isCreator, XLPQWORD xuid, XXOVERLAPPED* overlapped)
{
    if (isCreator)
        *isCreator = true;

    if (xuid)
        *xuid = 0xB13EBABEBABEBABE;

    return 0;
}

uint32_t XamContentGetDeviceState()
{
    return 0;
}

uint32_t XamUserGetSigninInfo(uint32_t userIndex, uint32_t flags, XUSER_SIGNIN_INFO* info)
{
    if (userIndex == 0)
    {
        memset(info, 0, sizeof(*info));
        info->xuid = 0xB13EBABEBABEBABE;
        info->SigninState = 1;
        strcpy(info->Name, "SWA");
        return 0;
    }

    return 0x00000525; // ERROR_NO_SUCH_USER
}

void XamShowSigninUI()
{
    LOG_UTILITY("!!! STUB !!!");
}

uint32_t XamShowDeviceSelectorUI
(
    uint32_t userIndex,
    uint32_t contentType,
    uint32_t contentFlags,
    uint64_t totalRequested,
    XDWORD* deviceId,
    XXOVERLAPPED* overlapped
)
{
    XamNotifyEnqueueEvent(9, true);
    *deviceId = 1;
    XamNotifyEnqueueEvent(9, false);
    return 0;
}

void XamShowDirtyDiscErrorUI()
{
    LOG_UTILITY("!!! STUB !!!");
}

void XamEnableInactivityProcessing()
{
    LOG_UTILITY("!!! STUB !!!");
}

void XamResetInactivity()
{
    LOG_UTILITY("!!! STUB !!!");
}

void XamShowMessageBoxUIEx()
{
    LOG_UTILITY("!!! STUB !!!");
}

uint32_t XGetLanguage()
{
    return (uint32_t)Config::Language.Value;
}

uint32_t XGetAVPack()
{
    return 0;
}

void XamLoaderTerminateTitle()
{
    LOG_UTILITY("!!! STUB !!!");
}

void XamGetExecutionId()
{
    LOG_UTILITY("!!! STUB !!!");
}

void XamLoaderLaunchTitle()
{
    LOG_UTILITY("!!! STUB !!!");
}

void NtOpenFile()
{
    LOG_UTILITY("!!! STUB !!!");
}

void RtlInitAnsiString(XANSI_STRING* destination, char* source)
{
    const uint16_t length = source ? (uint16_t)strlen(source) : 0;
    destination->Length = length;
    destination->MaximumLength = length + 1;
    destination->Buffer = source;
}

DWORD NtCreateFile
(
    XLPDWORD FileHandle,
    DWORD DesiredAccess,
    XOBJECT_ATTRIBUTES* Attributes,
    XIO_STATUS_BLOCK* IoStatusBlock,
    uint64_t* AllocationSize,
    uint32_t FileAttributes,
    uint32_t ShareAccess,
    uint32_t CreateDisposition,
    uint32_t CreateOptions
)
{
    return 0;
}

uint32_t NtClose(uint32_t handle)
{
    if (handle == GUEST_INVALID_HANDLE_VALUE)
        return 0xFFFFFFFF;

    if (IsKernelObject(handle))
    {
        DestroyKernelObject(handle);
        return 0;
    }

    return CloseHandle((HANDLE)handle) ? 0 : 0xFFFFFFFF;
}

void NtSetInformationFile()
{
    LOG_UTILITY("!!! STUB !!!");
}

uint32_t FscSetCacheElementCount()
{
    return 0;
}

DWORD NtWaitForSingleObjectEx(DWORD Handle, DWORD WaitMode, DWORD Alertable, XLPQWORD Timeout)
{
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
    LOG_UTILITY("!!! STUB !!!");
}

void vsprintf_x()
{
    LOG_UTILITY("!!! STUB !!!");
}

uint32_t ExGetXConfigSetting(uint16_t Category, uint16_t Setting, void* Buffer, uint16_t SizeOfBuffer, XLPDWORD RequiredSize)
{
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
                    data[0] = ByteSwap(0x00001000); // USA/Canada
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
                    data[0] = ByteSwap((uint32_t)Config::Language.Value);
                    break;

                // XCONFIG_USER_VIDEO_FLAGS
                case 0x000A:
                    data[0] = ByteSwap(0x00040000);
                    break;

                // XCONFIG_USER_RETAIL_FLAGS
                case 0x000C:
                    data[0] = ByteSwap(1);
                    break;

                // XCONFIG_USER_COUNTRY
                case 0x000E:
                    data[0] = ByteSwap(103);
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
    LOG_UTILITY("!!! STUB !!!");
}

void MmQueryStatistics()
{
    LOG_UTILITY("!!! STUB !!!");
}

uint32_t NtCreateEvent(uint32_t* handle, void* objAttributes, uint32_t eventType, uint32_t initialState)
{
    *handle = ByteSwap((uint32_t)CreateEventA(nullptr, !eventType, !!initialState, nullptr));
    return 0;
}

uint32_t XexCheckExecutablePrivilege()
{
    return 0;
}

void DbgPrint()
{
    LOG_UTILITY("!!! STUB !!!");
}

void __C_specific_handler_x()
{
    LOG_UTILITY("!!! STUB !!!");
}

void RtlNtStatusToDosError()
{
    LOG_UTILITY("!!! STUB !!!");
}

void XexGetProcedureAddress()
{
    LOG_UTILITY("!!! STUB !!!");
}

void XexGetModuleSection()
{
    LOG_UTILITY("!!! STUB !!!");
}

NTSTATUS RtlUnicodeToMultiByteN(PCHAR MultiByteString, DWORD MaxBytesInMultiByteString, XLPDWORD BytesInMultiByteString, PCWCH UnicodeString, ULONG BytesInUnicodeString)
{
    const auto reqSize = BytesInUnicodeString / sizeof(wchar_t);

    if (BytesInMultiByteString)
        *BytesInMultiByteString = reqSize;

    if (reqSize > MaxBytesInMultiByteString)
        return STATUS_FAIL_CHECK;

    for (size_t i = 0; i < reqSize; i++)
    {
        const auto c = ByteSwap(UnicodeString[i]);

        MultiByteString[i] = c < 256 ? c : '?';
    }

    return STATUS_SUCCESS;
}

DWORD KeDelayExecutionThread(DWORD WaitMode, bool Alertable, XLPQWORD Timeout)
{
    // We don't do async file reads.
    if (Alertable)
        return STATUS_USER_APC;

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
    LOG_UTILITY("!!! STUB !!!");
}

void NtQueryInformationFile()
{
    LOG_UTILITY("!!! STUB !!!");
}

void NtQueryVolumeInformationFile()
{
    LOG_UTILITY("!!! STUB !!!");
}

void NtQueryDirectoryFile()
{
    LOG_UTILITY("!!! STUB !!!");
}

void NtReadFileScatter()
{
    LOG_UTILITY("!!! STUB !!!");
}

void NtReadFile()
{
    LOG_UTILITY("!!! STUB !!!");
}

void NtDuplicateObject()
{
    LOG_UTILITY("!!! STUB !!!");
}

void NtAllocateVirtualMemory()
{
    LOG_UTILITY("!!! STUB !!!");
}

void NtFreeVirtualMemory()
{
    LOG_UTILITY("!!! STUB !!!");
}

void ObDereferenceObject()
{
    LOG_UTILITY("!!! STUB !!!");
}

void KeSetBasePriorityThread(uint32_t thread, int priority)
{
    if (priority == 16)
    {
        priority = 15;
    }
    else if (priority == -16)
    {
        priority = -15;
    }

    SetThreadPriority((HANDLE)thread, priority);
}

uint32_t ObReferenceObjectByHandle(uint32_t handle, uint32_t objectType, XLPDWORD object)
{
    *object = handle;
    return 0;
}

void KeQueryBasePriorityThread()
{
    LOG_UTILITY("!!! STUB !!!");
}

uint32_t NtSuspendThread(uint32_t hThread, uint32_t* suspendCount)
{
    DWORD count = SuspendThread((HANDLE)hThread);

    if (count == (DWORD)-1)
        return E_FAIL;

    if (suspendCount != nullptr)
        *suspendCount = ByteSwap(count);

    return S_OK;
}

uint32_t KeSetAffinityThread(DWORD Thread, DWORD Affinity, XLPDWORD lpPreviousAffinity)
{
    if (lpPreviousAffinity)
        *lpPreviousAffinity = 2;

    return 0;
}

struct Event : KernelObject, HostObject<XKEVENT>
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

struct Semaphore : KernelObject, HostObject<XKSEMAPHORE>
{
    HANDLE handle;

    Semaphore(XKSEMAPHORE* semaphore)
    {
        handle = CreateSemaphoreA(nullptr, semaphore->Header.SignalState, semaphore->Limit, nullptr);
    }
};

void RtlLeaveCriticalSection(XRTL_CRITICAL_SECTION* cs)
{
    cs->RecursionCount--;

    if (cs->RecursionCount != 0)
        return;

    std::atomic_ref owningThread(cs->OwningThread);
    owningThread.store(0);
    owningThread.notify_one();
}

void RtlEnterCriticalSection(XRTL_CRITICAL_SECTION* cs)
{
    uint32_t thisThread = g_ppcContext->r13.u32;
    assert(thisThread != NULL);

    std::atomic_ref owningThread(cs->OwningThread);

    while (true) 
    {
        uint32_t previousOwner = 0;

        if (owningThread.compare_exchange_weak(previousOwner, thisThread) || previousOwner == thisThread)
        {
            cs->RecursionCount++;
            return;
        }

        owningThread.wait(previousOwner);
    }
}

void RtlImageXexHeaderField()
{
    LOG_UTILITY("!!! STUB !!!");
}

void HalReturnToFirmware()
{
    LOG_UTILITY("!!! STUB !!!");
}

void RtlFillMemoryUlong()
{
    LOG_UTILITY("!!! STUB !!!");
}

void KeBugCheckEx()
{
    __debugbreak();
}

uint32_t KeGetCurrentProcessType()
{
    return 1;
}

void RtlCompareMemoryUlong()
{
    LOG_UTILITY("!!! STUB !!!");
}

uint32_t RtlInitializeCriticalSection(XRTL_CRITICAL_SECTION* cs)
{
    cs->Header.Absolute = 0;
    cs->LockCount = -1;
    cs->RecursionCount = 0;
    cs->OwningThread = 0;

    return 0;
}

void RtlRaiseException_x()
{
    LOG_UTILITY("!!! STUB !!!");
}

void KfReleaseSpinLock(uint32_t* spinLock)
{
    InterlockedExchange((volatile long*)spinLock, 0);
}

void KfAcquireSpinLock(uint32_t* spinLock)
{
    const auto ctx = GetPPCContext();

    while (InterlockedCompareExchange((volatile long*)spinLock, ByteSwap(*(uint32_t*)(g_memory.Translate(ctx->r13.u32 + 0x110))), 0) != 0)
        Sleep(0);
}

uint64_t KeQueryPerformanceFrequency()
{
    return 49875000;
}

void MmFreePhysicalMemory(uint32_t type, uint32_t guestAddress)
{
    if (guestAddress != NULL)
        g_userHeap.Free(g_memory.Translate(guestAddress));
}

bool VdPersistDisplay(uint32_t a1, uint32_t* a2)
{
    *a2 = NULL;
    return false;
}

void VdSwap()
{
    LOG_UTILITY("!!! STUB !!!");
}

void VdGetSystemCommandBuffer()
{
    LOG_UTILITY("!!! STUB !!!");
}

void KeReleaseSpinLockFromRaisedIrql(uint32_t* spinLock)
{
    InterlockedExchange((volatile long*)spinLock, 0);
}

void KeAcquireSpinLockAtRaisedIrql(uint32_t* spinLock)
{
    const auto ctx = GetPPCContext();

    while (InterlockedCompareExchange((volatile long*)spinLock, ByteSwap(*(uint32_t*)(g_memory.Translate(ctx->r13.u32 + 0x110))), 0) != 0)
        Sleep(0);
}

uint32_t KiApcNormalRoutineNop()
{
    return 0;
}

void VdEnableRingBufferRPtrWriteBack()
{
    LOG_UTILITY("!!! STUB !!!");
}

void VdInitializeRingBuffer()
{
    LOG_UTILITY("!!! STUB !!!");
}

uint32_t MmGetPhysicalAddress(uint32_t address)
{
    LOGF_UTILITY("0x{:x}", address);
    return address;
}

void VdSetSystemCommandBufferGpuIdentifierAddress()
{
    LOG_UTILITY("!!! STUB !!!");
}

void _vsnprintf_x()
{
    LOG_UTILITY("!!! STUB !!!");
}

void sprintf_x()
{
    LOG_UTILITY("!!! STUB !!!");
}

void ExRegisterTitleTerminateNotification()
{
    LOG_UTILITY("!!! STUB !!!");
}

void VdShutdownEngines()
{
    LOG_UTILITY("!!! STUB !!!");
}

void VdQueryVideoMode(XVIDEO_MODE* vm)
{
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
    LOG_UTILITY("!!! STUB !!!");
}

void VdSetDisplayMode()
{
    LOG_UTILITY("!!! STUB !!!");
}

void VdSetGraphicsInterruptCallback()
{
    LOG_UTILITY("!!! STUB !!!");
}

void VdInitializeEngines()
{
    LOG_UTILITY("!!! STUB !!!");
}

void VdIsHSIOTrainingSucceeded()
{
    LOG_UTILITY("!!! STUB !!!");
}

void VdGetCurrentDisplayGamma()
{
    LOG_UTILITY("!!! STUB !!!");
}

void VdQueryVideoFlags()
{
    LOG_UTILITY("!!! STUB !!!");
}

void VdCallGraphicsNotificationRoutines()
{
    LOG_UTILITY("!!! STUB !!!");
}

void VdInitializeScalerCommandBuffer()
{
    LOG_UTILITY("!!! STUB !!!");
}

void KeLeaveCriticalRegion()
{
    LOG_UTILITY("!!! STUB !!!");
}

uint32_t VdRetrainEDRAM()
{
    return 0;
}

void VdRetrainEDRAMWorker()
{
    LOG_UTILITY("!!! STUB !!!");
}

void KeEnterCriticalRegion()
{
    LOG_UTILITY("!!! STUB !!!");
}

uint32_t MmAllocatePhysicalMemoryEx
(
    uint32_t flags,
    uint32_t size,
    uint32_t protect,
    uint32_t minAddress,
    uint32_t maxAddress,
    uint32_t alignment
)
{
    LOGF_UTILITY("0x{:x}, 0x{:x}, 0x{:x}, 0x{:x}, 0x{:x}, 0x{:x}", flags, size, protect, minAddress, maxAddress, alignment);
    return g_memory.MapVirtual(g_userHeap.AllocPhysical(size, alignment));
}

void ObDeleteSymbolicLink()
{
    LOG_UTILITY("!!! STUB !!!");
}

void ObCreateSymbolicLink()
{
    LOG_UTILITY("!!! STUB !!!");
}

uint32_t MmQueryAddressProtect(uint32_t guestAddress)
{
    return PAGE_READWRITE;
}

void VdEnableDisableClockGating()
{
    LOG_UTILITY("!!! STUB !!!");
}

void KeBugCheck()
{
    __debugbreak();
}

void KeLockL2()
{
    LOG_UTILITY("!!! STUB !!!");
}

void KeUnlockL2()
{
    LOG_UTILITY("!!! STUB !!!");
}

bool KeSetEvent(XKEVENT* pEvent, DWORD Increment, bool Wait)
{
    return QueryKernelObject<Event>(*pEvent)->Set();
}

bool KeResetEvent(XKEVENT* pEvent)
{
    return QueryKernelObject<Event>(*pEvent)->Reset();
}

DWORD KeWaitForSingleObject(XDISPATCHER_HEADER* Object, DWORD WaitReason, DWORD WaitMode, bool Alertable, XLPQWORD Timeout)
{
    const uint64_t timeout = GuestTimeoutToMilliseconds(Timeout);

    HANDLE handle = nullptr;

    switch (Object->Type)
    {
        case 0:
        case 1:
            handle = QueryKernelObject<Event>(*Object)->handle;
            break;

        case 5:
            handle = QueryKernelObject<Semaphore>(*Object)->handle;
            break;

        default:
            assert(false);
            break;
    }

    return WaitForSingleObjectEx(handle, timeout, Alertable);
}

static thread_local std::vector<uint32_t> KeTlsValues;
static std::vector<size_t> KeTlsFreeIndices;
static size_t KeTlsNextIndex = 0;
static Mutex KeTlsAllocationMutex;

static void KeTlsEnsureTlsCapacity(size_t index)
{
    if (KeTlsValues.size() <= index)
    {
        KeTlsValues.resize(index + 1, 0);
    }
}

uint32_t KeTlsGetValue(DWORD dwTlsIndex)
{
    KeTlsEnsureTlsCapacity(dwTlsIndex);
    return KeTlsValues[dwTlsIndex];
}

BOOL KeTlsSetValue(DWORD dwTlsIndex, DWORD lpTlsValue)
{
    KeTlsEnsureTlsCapacity(dwTlsIndex);
    KeTlsValues[dwTlsIndex] = lpTlsValue;
    return TRUE;
}

DWORD KeTlsAlloc()
{
    std::lock_guard<Mutex> lock(KeTlsAllocationMutex);
    if (!KeTlsFreeIndices.empty())
    {
        size_t index = KeTlsFreeIndices.back();
        KeTlsFreeIndices.pop_back();
        return index;
    }

    return KeTlsNextIndex++;
}

BOOL KeTlsFree(DWORD dwTlsIndex)
{
    std::lock_guard<Mutex> lock(KeTlsAllocationMutex);
    KeTlsFreeIndices.push_back(dwTlsIndex);
    return TRUE;
}

DWORD XMsgInProcessCall(uint32_t app, uint32_t message, XDWORD* param1, XDWORD* param2)
{
    if (message == 0x7001B)
    {
        uint32_t* ptr = (uint32_t*)g_memory.Translate(param1[1]);
        ptr[0] = 0;
        ptr[1] = 0;
    }

    return 0;
}

void XamUserReadProfileSettings
(
    uint32_t titleId,
    uint32_t userIndex,
    uint32_t xuidCount,
    uint64_t* xuids,
    uint32_t settingCount,
    uint32_t* settingIds,
    XDWORD* bufferSize,
    void* buffer,
    void* overlapped
)
{
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
    LOG_UTILITY("!!! STUB !!!");
}

void NetDll_WSACleanup()
{
    LOG_UTILITY("!!! STUB !!!");
}

void NetDll_socket()
{
    LOG_UTILITY("!!! STUB !!!");
}

void NetDll_closesocket()
{
    LOG_UTILITY("!!! STUB !!!");
}

void NetDll_setsockopt()
{
    LOG_UTILITY("!!! STUB !!!");
}

void NetDll_bind()
{
    LOG_UTILITY("!!! STUB !!!");
}

void NetDll_connect()
{
    LOG_UTILITY("!!! STUB !!!");
}

void NetDll_listen()
{
    LOG_UTILITY("!!! STUB !!!");
}

void NetDll_accept()
{
    LOG_UTILITY("!!! STUB !!!");
}

void NetDll_select()
{
    LOG_UTILITY("!!! STUB !!!");
}

void NetDll_recv()
{
    LOG_UTILITY("!!! STUB !!!");
}

void NetDll_send()
{
    LOG_UTILITY("!!! STUB !!!");
}

void NetDll_inet_addr()
{
    LOG_UTILITY("!!! STUB !!!");
}

void NetDll___WSAFDIsSet()
{
    LOG_UTILITY("!!! STUB !!!");
}

void XMsgStartIORequestEx()
{
    LOG_UTILITY("!!! STUB !!!");
}

void XexGetModuleHandle()
{
    LOG_UTILITY("!!! STUB !!!");
}

bool RtlTryEnterCriticalSection(XRTL_CRITICAL_SECTION* cs)
{
    uint32_t thisThread = g_ppcContext->r13.u32;
    assert(thisThread != NULL);

    std::atomic_ref owningThread(cs->OwningThread);

    uint32_t previousOwner = 0;

    if (owningThread.compare_exchange_weak(previousOwner, thisThread) || previousOwner == thisThread)
    {
        cs->RecursionCount++;
        return true;
    }

    return false;
}

void RtlInitializeCriticalSectionAndSpinCount(XRTL_CRITICAL_SECTION* cs, uint32_t spinCount)
{
    cs->Header.Absolute = (spinCount + 255) >> 8;
    cs->LockCount = -1;
    cs->RecursionCount = 0;
    cs->OwningThread = 0;
}

void _vswprintf_x()
{
    LOG_UTILITY("!!! STUB !!!");
}

void _vscwprintf_x()
{
    LOG_UTILITY("!!! STUB !!!");
}

void _swprintf_x()
{
    LOG_UTILITY("!!! STUB !!!");
}

void _snwprintf_x()
{
    LOG_UTILITY("!!! STUB !!!");
}

void XeCryptBnQwBeSigVerify()
{
    LOG_UTILITY("!!! STUB !!!");
}

void XeKeysGetKey()
{
    LOG_UTILITY("!!! STUB !!!");
}

void XeCryptRotSumSha()
{
    LOG_UTILITY("!!! STUB !!!");
}

void XeCryptSha()
{
    LOG_UTILITY("!!! STUB !!!");
}

void KeEnableFpuExceptions()
{
    LOG_UTILITY("!!! STUB !!!");
}

void RtlUnwind_x()
{
    LOG_UTILITY("!!! STUB !!!");
}

void RtlCaptureContext_x()
{
    LOG_UTILITY("!!! STUB !!!");
}

void NtQueryFullAttributesFile()
{
    LOG_UTILITY("!!! STUB !!!");
}

NTSTATUS RtlMultiByteToUnicodeN(PWCH UnicodeString, ULONG MaxBytesInUnicodeString, XLPDWORD BytesInUnicodeString, const CHAR* MultiByteString, ULONG BytesInMultiByteString)
{
    // i am lazy
    const auto n = MultiByteToWideChar(CP_UTF8, 0, MultiByteString, BytesInMultiByteString, UnicodeString, MaxBytesInUnicodeString);

    if (BytesInUnicodeString)
        *BytesInUnicodeString = n * sizeof(wchar_t);

    if (n)
    {
        for (size_t i = 0; i < n; i++)
            UnicodeString[i] = ByteSwap(UnicodeString[i]);
    }

    return STATUS_SUCCESS;
}

void DbgBreakPoint()
{
    LOG_UTILITY("!!! STUB !!!");
}

void MmQueryAllocationSize()
{
    LOG_UTILITY("!!! STUB !!!");
}

uint32_t NtClearEvent(uint32_t handle, uint32_t* previousState)
{
    return ResetEvent((HANDLE)handle) ? 0 : 0xFFFFFFFF;
}

uint32_t NtResumeThread(uint32_t hThread, uint32_t* suspendCount)
{
    DWORD count = ResumeThread((HANDLE)hThread);

    if (count == (DWORD)-1)
        return E_FAIL;

    if (suspendCount != nullptr)
        *suspendCount = ByteSwap(count);

    return S_OK;
}

uint32_t NtSetEvent(uint32_t handle, uint32_t* previousState)
{
    return SetEvent((HANDLE)handle) ? 0 : 0xFFFFFFFF;
}

NTSTATUS NtCreateSemaphore(XLPDWORD Handle, XOBJECT_ATTRIBUTES* ObjectAttributes, DWORD InitialCount, DWORD MaximumCount)
{
    *Handle = (uint32_t)CreateSemaphoreA(nullptr, InitialCount, MaximumCount, nullptr);
    return STATUS_SUCCESS;
}

NTSTATUS NtReleaseSemaphore(uint32_t Handle, DWORD ReleaseCount, LONG* PreviousCount)
{
    ReleaseSemaphore((HANDLE)Handle, ReleaseCount, PreviousCount);

    if (PreviousCount)
        *PreviousCount = ByteSwap(*PreviousCount);

    return STATUS_SUCCESS;
}

void NtWaitForMultipleObjectsEx()
{
    LOG_UTILITY("!!! STUB !!!");
}

void RtlCompareStringN()
{
    LOG_UTILITY("!!! STUB !!!");
}

void _snprintf_x()
{
    LOG_UTILITY("!!! STUB !!!");
}

void StfsControlDevice()
{
    LOG_UTILITY("!!! STUB !!!");
}

void StfsCreateDevice()
{
    LOG_UTILITY("!!! STUB !!!");
}

void NtFlushBuffersFile()
{
    LOG_UTILITY("!!! STUB !!!");
}

void KeQuerySystemTime(uint64_t* time)
{
    FILETIME t;
    GetSystemTimeAsFileTime(&t);
    *time = ByteSwap((uint64_t(t.dwHighDateTime) << 32) | t.dwLowDateTime);
}

void RtlTimeToTimeFields()
{
    LOG_UTILITY("!!! STUB !!!");
}

void RtlFreeAnsiString()
{
    LOG_UTILITY("!!! STUB !!!");
}

void RtlUnicodeStringToAnsiString()
{
    LOG_UTILITY("!!! STUB !!!");
}

void RtlInitUnicodeString()
{
    LOG_UTILITY("!!! STUB !!!");
}

void ExTerminateThread()
{
    LOG_UTILITY("!!! STUB !!!");
}

uint32_t ExCreateThread(XLPDWORD handle, uint32_t stackSize, XLPDWORD threadId, uint32_t xApiThreadStartup, uint32_t startAddress, uint32_t startContext, uint32_t creationFlags)
{
    LOGF_UTILITY("0x{:X}, 0x{:X}, 0x{:X}, 0x{:X}, 0x{:X}, 0x{:X}, 0x{:X}",
        (intptr_t)handle, stackSize, (intptr_t)threadId, xApiThreadStartup, startAddress, startContext, creationFlags);

    DWORD hostThreadId;

    *handle = (uint32_t)GuestThread::Start(startAddress, startContext, creationFlags, &hostThreadId);

    if (threadId != nullptr)
        *threadId = hostThreadId;

    return 0;
}

void IoInvalidDeviceRequest()
{
    LOG_UTILITY("!!! STUB !!!");
}

void ObReferenceObject()
{
    LOG_UTILITY("!!! STUB !!!");
}

void IoCreateDevice()
{
    LOG_UTILITY("!!! STUB !!!");
}

void IoDeleteDevice()
{
    LOG_UTILITY("!!! STUB !!!");
}

void ExAllocatePoolTypeWithTag()
{
    LOG_UTILITY("!!! STUB !!!");
}

void RtlTimeFieldsToTime()
{
    LOG_UTILITY("!!! STUB !!!");
}

void IoCompleteRequest()
{
    LOG_UTILITY("!!! STUB !!!");
}

void RtlUpcaseUnicodeChar()
{
    LOG_UTILITY("!!! STUB !!!");
}

void ObIsTitleObject()
{
    LOG_UTILITY("!!! STUB !!!");
}

void IoCheckShareAccess()
{
    LOG_UTILITY("!!! STUB !!!");
}

void IoSetShareAccess()
{
    LOG_UTILITY("!!! STUB !!!");
}

void IoRemoveShareAccess()
{
    LOG_UTILITY("!!! STUB !!!");
}

void NetDll_XNetStartup()
{
    LOG_UTILITY("!!! STUB !!!");
}

void NetDll_XNetGetTitleXnAddr()
{
    LOG_UTILITY("!!! STUB !!!");
}

DWORD KeWaitForMultipleObjects(DWORD Count, xpointer<XDISPATCHER_HEADER>* Objects, DWORD WaitType, DWORD WaitReason, DWORD WaitMode, DWORD Alertable, XLPQWORD Timeout)
{
    // TODO: create actual objects by type.
    const uint64_t timeout = GuestTimeoutToMilliseconds(Timeout);

    thread_local std::vector<HANDLE> events;
    events.resize(Count);

    for (size_t i = 0; i < Count; i++)
    {
        assert(Objects[i]->Type <= 1);
        events[i] = QueryKernelObject<Event>(*Objects[i].get())->handle;
    }

    return WaitForMultipleObjectsEx(Count, events.data(), WaitType == 0, timeout, Alertable);
}

uint32_t KeRaiseIrqlToDpcLevel()
{
    return 0;
}

void KfLowerIrql() { }

uint32_t KeReleaseSemaphore(XKSEMAPHORE* semaphore, uint32_t increment, uint32_t adjustment, uint32_t wait)
{
    auto* object = QueryKernelObject<Semaphore>(semaphore->Header);
    return ReleaseSemaphore(object->handle, adjustment, nullptr) ? 0 : 0xFFFFFFFF;
}

void XAudioGetVoiceCategoryVolume()
{
    LOG_UTILITY("!!! STUB !!!");
}

DWORD XAudioGetVoiceCategoryVolumeChangeMask(DWORD Driver, XLPDWORD Mask)
{
    *Mask = 0;
    return 0;
}

uint32_t KeResumeThread(uint32_t object)
{
    LOGF_UTILITY("0x{:x}", object);
    return ResumeThread((HANDLE)object);
}

void KeInitializeSemaphore(XKSEMAPHORE* semaphore, uint32_t count, uint32_t limit)
{
    semaphore->Header.Type = 5;
    semaphore->Header.SignalState = count;
    semaphore->Limit = limit;

    auto* object = QueryKernelObject<Semaphore>(semaphore->Header);
}

void XMAReleaseContext()
{
    LOG_UTILITY("!!! STUB !!!");
}

void XMACreateContext()
{
    LOG_UTILITY("!!! STUB !!!");
}

// uint32_t XAudioRegisterRenderDriverClient(XLPDWORD callback, XLPDWORD driver)
// {
//     //printf("XAudioRegisterRenderDriverClient(): %x %x\n");
// 
//     *driver = apu::RegisterClient(callback[0], callback[1]);
//     return 0;
// }

// void XAudioUnregisterRenderDriverClient()
// {
//     printf("!!! STUB !!! XAudioUnregisterRenderDriverClient\n");
// }

// uint32_t XAudioSubmitRenderDriverFrame(uint32_t driver, void* samples)
// {
//     // printf("!!! STUB !!! XAudioSubmitRenderDriverFrame\n");
//     apu::SubmitFrames(samples);
// 
//     return 0;
// }

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
