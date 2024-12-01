#include "sdl2_driver.h"
#include <cpu/code_cache.h>
#include <cpu/guest_thread.h>
#include <cpu/guest_code.h>
#include <kernel/heap.h>

#define SDLAUDIO_DRIVER_KEY (uint32_t)('SDLA')

static constexpr uint32_t AUDIO_FRAME_SIZE = XAUDIO_NUM_SAMPLES * XAUDIO_NUM_CHANNELS;

static std::atomic<PPCFunc*> g_clientCallback{};
static DWORD g_clientCallbackParam{}; // pointer in guest memory

static SDL_AudioDeviceID g_audioDevice{};
static moodycamel::BlockingReaderWriterCircularBuffer<std::array<uint32_t, AUDIO_FRAME_SIZE>> g_audioQueue(16);

static void SDLAudioCallback(void*, uint8_t* frames, int len)
{
    std::array<uint32_t, AUDIO_FRAME_SIZE> audioFrame;
    if (g_audioQueue.try_dequeue(audioFrame))
        memcpy(frames, &audioFrame, sizeof(audioFrame));
    else
        memset(frames, 0, len);
}

static PPC_FUNC(DriverLoop)
{
    GuestThread::SetThreadName(GetCurrentThreadId(), "Audio Driver");

    while (true)
    {
        if (!g_clientCallback)
            continue;

        ctx.r3.u64 = g_clientCallbackParam;
        GuestCode::Run((void*)g_clientCallback, &ctx);
    }
}

void XAudioInitializeSystem()
{
    SDL_SetHint(SDL_HINT_AUDIO_CATEGORY, "playback");
    SDL_SetHint(SDL_HINT_AUDIO_DEVICE_APP_NAME, "SWA");

    auto err = SDL_InitSubSystem(SDL_INIT_AUDIO);
    SDL_AudioSpec spec{};
    spec.freq = XAUDIO_SAMPLES_HZ;
    spec.format = AUDIO_F32SYS;
    spec.channels = XAUDIO_NUM_CHANNELS;
    spec.samples = XAUDIO_NUM_SAMPLES;
    spec.callback = SDLAudioCallback;
    g_audioDevice = SDL_OpenAudioDevice(nullptr, false, &spec, &spec, 0);
    assert(g_audioDevice);

    SDL_PauseAudioDevice(g_audioDevice, 0);
    KeInsertHostFunction(SDLAUDIO_DRIVER_KEY, DriverLoop);
    GuestThread::Start(SDLAUDIO_DRIVER_KEY, 0, 0, nullptr);
}

void XAudioRegisterClient(PPCFunc* callback, uint32_t param)
{
    auto* pClientParam = static_cast<uint32_t*>(g_userHeap.Alloc(sizeof(param)));
    ByteSwap(param);
    *pClientParam = param;
    g_clientCallbackParam = g_memory.MapVirtual(pClientParam);

    g_clientCallback = callback;
}

void XAudioSubmitFrame(void* samples)
{
    std::array<uint32_t, AUDIO_FRAME_SIZE> audioFrame;

    for (size_t i = 0; i < XAUDIO_NUM_SAMPLES; i++)
    {
        for (size_t j = 0; j < XAUDIO_NUM_CHANNELS; j++)
            audioFrame[i * XAUDIO_NUM_CHANNELS + j] = std::byteswap(((uint32_t*)samples)[j * XAUDIO_NUM_SAMPLES + i]);
    }

    g_audioQueue.wait_enqueue(audioFrame);
}
