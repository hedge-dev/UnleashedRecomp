#pragma once

#define XAUDIO_SAMPLES_HZ 48000
#define XAUDIO_NUM_CHANNELS 6
#define XAUDIO_SAMPLE_BITS 32

// Number of samples in a frame
#define XAUDIO_NUM_SAMPLES 256

#ifdef SWA_IMPL
void XAudioInitializeSystem();
void XAudioRegisterClient(PPCFunc* callback, uint32_t param);
void XAudioSubmitFrame(void* samples);
#endif

uint32_t XAudioRegisterRenderDriverClient(be<uint32_t>* callback, be<uint32_t>* driver);
uint32_t XAudioUnregisterRenderDriverClient(uint32_t driver);
uint32_t XAudioSubmitRenderDriverFrame(uint32_t driver, void* samples);
