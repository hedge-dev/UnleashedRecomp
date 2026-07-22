#pragma once

#include <condition_variable>
#include <kernel/xdm.h>

// Use pthreads directly on macOS to be able to increase default stack size.
#ifdef __APPLE__
#define USE_PTHREAD 1
#endif

#ifdef USE_PTHREAD
#include <pthread.h>
#endif

#define CURRENT_THREAD_HANDLE uint32_t(-2)

struct GuestThreadContext
{
    PPCContext ppcContext{};
    uint8_t* thread = nullptr;

    GuestThreadContext(uint32_t cpuNumber);
    ~GuestThreadContext();
};

struct GuestThreadParams
{
    uint32_t function;
    uint32_t value;
    uint32_t flags;
};

struct GuestThreadHandle : KernelObject
{
    GuestThreadParams params;
    std::atomic<bool> suspended;
#if defined(UNLEASHED_RECOMP_IOS_LAUNCHER)
    std::mutex suspendMutex;
    std::condition_variable suspendCondition;
#endif
#ifdef USE_PTHREAD
    pthread_t thread;
#else
    std::thread thread;
#endif

    GuestThreadHandle(const GuestThreadParams& params);
    ~GuestThreadHandle() override;

    uint32_t GetThreadId() const;
    void SetSuspended(bool value);
    void WaitWhileSuspended();

    uint32_t Wait(uint32_t timeout) override;
};

struct GuestThread
{
    static uint32_t Start(const GuestThreadParams& params);
    static GuestThreadHandle* Start(const GuestThreadParams& params, uint32_t* threadId);

    static uint32_t GetCurrentThreadId();
    static uint32_t FindHostThreadId(uint32_t guestThreadId);
    static void SetLastError(uint32_t error);

#ifdef _WIN32
    static void SetThreadName(uint32_t threadId, const char* name);
#endif
};
