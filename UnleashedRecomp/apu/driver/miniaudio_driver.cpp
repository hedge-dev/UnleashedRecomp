#include "miniaudio_driver.h"
#include <cpu/code_cache.h>
#include <cpu/guest_thread.h>
#include <cpu/guest_code.h>
#include <kernel/heap.h>

#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio.h>

static PPCFunc* g_clientCallback{};
static DWORD g_clientCallbackParam{}; // pointer in guest memory
static ma_device g_audioDevice{};
static std::unique_ptr<GuestThreadContext> g_audioCtx;
static uint32_t* g_audioOutput;

static void AudioCallback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount)
{
    if (g_audioCtx == nullptr)
        g_audioCtx = std::make_unique<GuestThreadContext>(0);

    g_audioCtx->ppcContext.r3.u64 = g_clientCallbackParam;
    g_audioOutput = reinterpret_cast<uint32_t*>(pOutput);
    (*g_clientCallback)(g_audioCtx->ppcContext, reinterpret_cast<uint8_t*>(g_memory.base));
}

void XAudioInitializeSystem()
{
    ma_device_config deviceConfig = ma_device_config_init(ma_device_type_playback);
    deviceConfig.sampleRate = XAUDIO_SAMPLES_HZ;
    deviceConfig.periodSizeInFrames = XAUDIO_NUM_SAMPLES;
    deviceConfig.noPreSilencedOutputBuffer = true;
    deviceConfig.dataCallback = AudioCallback;
    deviceConfig.playback.format = ma_format_f32;
    deviceConfig.playback.channels = XAUDIO_NUM_CHANNELS;
    ma_device_init(nullptr, &deviceConfig, &g_audioDevice);
}

void XAudioRegisterClient(PPCFunc* callback, uint32_t param)
{
    auto* pClientParam = static_cast<uint32_t*>(g_userHeap.Alloc(sizeof(param)));
    ByteSwap(param);
    *pClientParam = param;
    g_clientCallbackParam = g_memory.MapVirtual(pClientParam);
    g_clientCallback = callback;

    ma_device_start(&g_audioDevice);
}

void XAudioSubmitFrame(void* samples)
{
    for (size_t i = 0; i < XAUDIO_NUM_SAMPLES; i++)
    {
        for (size_t j = 0; j < XAUDIO_NUM_CHANNELS; j++)
            g_audioOutput[i * XAUDIO_NUM_CHANNELS + j] = std::byteswap(((uint32_t*)samples)[j * XAUDIO_NUM_SAMPLES + i]);
    }
}
