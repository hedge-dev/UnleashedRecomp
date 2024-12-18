#include "sdl2_driver.h"
#include <cpu/code_cache.h>
#include <cpu/guest_thread.h>
#include <cpu/guest_code.h>
#include <kernel/heap.h>

static PPCFunc* g_clientCallback{};
static uint32_t g_clientCallbackParam{}; // pointer in guest memory
static SDL_AudioDeviceID g_audioDevice{};
static std::unique_ptr<GuestThreadContext> g_audioCtx;
static uint32_t* g_audioOutput;
static bool g_downMixToStereo;

static void AudioCallback(void*, uint8_t* frames, int len)
{
    if (g_audioCtx == nullptr)
        g_audioCtx = std::make_unique<GuestThreadContext>(0);

    g_audioCtx->ppcContext.r3.u64 = g_clientCallbackParam;
    g_audioOutput = reinterpret_cast<uint32_t*>(frames);
    (*g_clientCallback)(g_audioCtx->ppcContext, reinterpret_cast<uint8_t*>(g_memory.base));
}

void XAudioInitializeSystem()
{
    SDL_SetHint(SDL_HINT_AUDIO_CATEGORY, "playback");
    SDL_SetHint(SDL_HINT_AUDIO_DEVICE_APP_NAME, "Unleashed Recompiled");
    SDL_InitSubSystem(SDL_INIT_AUDIO);

    SDL_AudioSpec desired{}, obtained{};
    desired.freq = XAUDIO_SAMPLES_HZ;
    desired.format = AUDIO_F32SYS;
    desired.channels = XAUDIO_NUM_CHANNELS;
    desired.samples = XAUDIO_NUM_SAMPLES;
    desired.callback = AudioCallback;
    g_audioDevice = SDL_OpenAudioDevice(nullptr, 0, &desired, &obtained, SDL_AUDIO_ALLOW_CHANNELS_CHANGE);

    if (obtained.channels != 2 && obtained.channels != XAUDIO_NUM_CHANNELS)
    {
        SDL_CloseAudioDevice(g_audioDevice);
        g_audioDevice = SDL_OpenAudioDevice(nullptr, 0, &desired, &obtained, 0);
    }

    g_downMixToStereo = (obtained.channels == 2);
}

void XAudioRegisterClient(PPCFunc* callback, uint32_t param)
{
    auto* pClientParam = static_cast<uint32_t*>(g_userHeap.Alloc(sizeof(param)));
    ByteSwapInplace(param);
    *pClientParam = param;
    g_clientCallbackParam = g_memory.MapVirtual(pClientParam);
    g_clientCallback = callback;

    SDL_PauseAudioDevice(g_audioDevice, 0);
}

void XAudioSubmitFrame(void* samples)
{
    if (g_downMixToStereo)
    {
        // 0: left 1.0f, right 0.0f
        // 1: left 0.0f, right 1.0f
        // 2: left 0.75f, right 0.75f
        // 3: left 0.0f, right 0.0f
        // 4: left 1.0f, right 0.0f
        // 5: left 0.0f, right 1.0f

        auto floatSamples = reinterpret_cast<be<float>*>(samples);

        for (size_t i = 0; i < XAUDIO_NUM_SAMPLES; i++)
        {
            float ch0 = floatSamples[0 * XAUDIO_NUM_SAMPLES + i];
            float ch1 = floatSamples[1 * XAUDIO_NUM_SAMPLES + i];
            float ch2 = floatSamples[2 * XAUDIO_NUM_SAMPLES + i];
            float ch3 = floatSamples[3 * XAUDIO_NUM_SAMPLES + i];
            float ch4 = floatSamples[4 * XAUDIO_NUM_SAMPLES + i];
            float ch5 = floatSamples[5 * XAUDIO_NUM_SAMPLES + i];

            float left  = ch0 + ch2 * 0.75f + ch4;
            float right = ch1 + ch2 * 0.75f + ch5;

            g_audioOutput[i * 2 + 0] = *reinterpret_cast<uint32_t*>(&left);
            g_audioOutput[i * 2 + 1] = *reinterpret_cast<uint32_t*>(&right);
        }
    }
    else
    {
        auto rawSamples = reinterpret_cast<be<uint32_t>*>(samples);

        for (size_t i = 0; i < XAUDIO_NUM_SAMPLES; i++)
        {
            for (size_t j = 0; j < XAUDIO_NUM_CHANNELS; j++)
                g_audioOutput[i * XAUDIO_NUM_CHANNELS + j] = rawSamples[j * XAUDIO_NUM_SAMPLES + i];
        }
    }
}
