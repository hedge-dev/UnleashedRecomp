#include <stdafx.h>
#include "xaudio_driver.h"
#include <xaudio2.h>
#include <cpu/code_cache.h>
#include <cpu/guest_thread.h>
#include <cpu/guest_code.h>
#include <cpu/ppc_context.h>
#include <kernel/heap.h>

#define XAUDIO_DRIVER_KEY (uint32_t)('XAUD')

PPCFunc* volatile g_clientCallback{};
DWORD g_clientCallbackParam{}; // pointer in guest memory
DWORD g_driverThread{};

// TODO: Should use a counted ptr
IXAudio2* g_audio{};
IXAudio2MasteringVoice* g_masteringVoice{};
IXAudio2SourceVoice* g_sourceVoice{};

constexpr uint32_t g_semaphoreCount = 16;
constexpr uint32_t g_audioFrameSize = 256 * 6;
HANDLE g_audioSemaphore{ CreateSemaphoreA(nullptr, g_semaphoreCount, g_semaphoreCount, nullptr) };
uint32_t g_audioFrames[g_audioFrameSize * g_semaphoreCount];
uint32_t g_audioFrameIndex = 0;

class VoiceCallback : public IXAudio2VoiceCallback
{
    STDMETHOD_(void, OnVoiceProcessingPassStart)(UINT32 BytesRequired) override {}
    STDMETHOD_(void, OnVoiceProcessingPassEnd)() override {}

    STDMETHOD_(void, OnBufferStart)(void* pBufferContext) override {}
    STDMETHOD_(void, OnBufferEnd)(void* pBufferContext) override
    {
        ReleaseSemaphore(g_audioSemaphore, 1, nullptr);
    }

    STDMETHOD_(void, OnStreamEnd)() override {}

    STDMETHOD_(void, OnLoopEnd)(void* pBufferContext) override {}
    STDMETHOD_(void, OnVoiceError)(void* pBufferContext, HRESULT Error) override {}
} gVoiceCallback;

PPC_FUNC(DriverLoop)
{
    GuestThread::SetThreadName(GetCurrentThreadId(), "Audio Driver");

    while (true)
    {
        if (!g_clientCallback)
        {
            continue;
        }

        WaitForSingleObject(g_audioSemaphore, INFINITE);

        ctx.r3.u64 = g_clientCallbackParam;
        GuestCode::Run((void*)g_clientCallback, &ctx);
    }
}

void XAudioInitializeSystem()
{
    if (g_audio)
    {
        return;
    }

    //reinterpret_cast<decltype(&XAudio2Create)>(
    //	GetProcAddress(LoadLibraryA("XAudio2_8.dll"), "XAudio2Create"))(&gAudio, 0, 1);

    XAudio2Create(&g_audio);
    g_audio->CreateMasteringVoice(&g_masteringVoice);

    WAVEFORMATIEEEFLOATEX format{};
    format.Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
    format.Format.cbSize = sizeof(format) - sizeof(format.Format);
    format.Format.nChannels = XAUDIO_NUM_CHANNELS;
    format.Format.nSamplesPerSec = XAUDIO_SAMPLES_HZ;
    format.Format.wBitsPerSample = XAUDIO_SAMPLE_BITS;
    format.Format.nBlockAlign = (format.Format.nChannels * format.Format.wBitsPerSample) / 8;
    format.Format.nAvgBytesPerSec = format.Format.nSamplesPerSec * format.Format.nBlockAlign;

    format.SubFormat = KSDATAFORMAT_SUBTYPE_IEEE_FLOAT;
    format.Samples.wValidBitsPerSample = format.Format.wBitsPerSample;
    format.dwChannelMask = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_CENTER | SPEAKER_FRONT_RIGHT |
        SPEAKER_LOW_FREQUENCY | SPEAKER_BACK_LEFT | SPEAKER_BACK_RIGHT;

    g_audio->CreateSourceVoice(&g_sourceVoice, &format.Format, 0, 1024, &gVoiceCallback);
    g_sourceVoice->Start();

    KeInsertHostFunction(XAUDIO_DRIVER_KEY, DriverLoop);
    GuestThread::Start(XAUDIO_DRIVER_KEY, 0, 0, &g_driverThread);
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
    uint32_t* audioFrame = &g_audioFrames[g_audioFrameSize * g_audioFrameIndex];
    g_audioFrameIndex = (g_audioFrameIndex + 1) % g_semaphoreCount;

    for (size_t i = 0; i < XAUDIO_NUM_SAMPLES; i++)
    {
        for (size_t j = 0; j < 6; j++)
            audioFrame[i * XAUDIO_NUM_CHANNELS + j] = std::byteswap(((uint32_t*)samples)[j * XAUDIO_NUM_SAMPLES + i]);
    }

    XAUDIO2_BUFFER buffer{};
    buffer.pAudioData = (BYTE*)audioFrame;
    buffer.AudioBytes = XAUDIO_NUM_SAMPLES * XAUDIO_NUM_CHANNELS * sizeof(float);
    buffer.PlayLength = XAUDIO_NUM_SAMPLES;

    g_sourceVoice->SubmitSourceBuffer(&buffer);
}
