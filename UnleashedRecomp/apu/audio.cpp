#include <stdafx.h>

#include <bit>

#include "audio.h"
#include "cpu/code_cache.h"

#define AUDIO_DRIVER_KEY (uint32_t)('DAUD')

// Use to dump raw audio captures to the game folder.
//#define AUDIO_DUMP_SAMPLES_PATH "audio.pcm"

#ifdef AUDIO_DUMP_SAMPLES_PATH
std::ofstream g_audioDumpStream;
#endif

uint32_t XAudioRegisterRenderDriverClient(XLPDWORD callback, XLPDWORD driver)
{
#ifdef AUDIO_DUMP_SAMPLES_PATH
    g_audioDumpStream.open(AUDIO_DUMP_SAMPLES_PATH, std::ios::binary);
#endif

    *driver = AUDIO_DRIVER_KEY;
    XAudioRegisterClient(KeFindHostFunction(*callback), callback[1]);
    return 0;
}

uint32_t XAudioUnregisterRenderDriverClient(DWORD driver)
{
    return 0;
}

uint32_t XAudioSubmitRenderDriverFrame(uint32_t driver, void* samples)
{
#ifdef AUDIO_DUMP_SAMPLES_PATH
    static uint32_t xaudioSamplesBuffer[XAUDIO_NUM_SAMPLES * XAUDIO_NUM_CHANNELS];
    for (size_t i = 0; i < XAUDIO_NUM_SAMPLES; i++)
    {
        for (size_t j = 0; j < XAUDIO_NUM_CHANNELS; j++)
        {
            xaudioSamplesBuffer[i * XAUDIO_NUM_CHANNELS + j] = std::byteswap(((uint32_t *)samples)[j * XAUDIO_NUM_SAMPLES + i]);
        }
    }

    g_audioDumpStream.write((const char *)(xaudioSamplesBuffer), sizeof(xaudioSamplesBuffer));
#endif

    XAudioSubmitFrame(samples);
    return 0;
}
