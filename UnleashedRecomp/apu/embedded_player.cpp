#include <miniaudio.h>

#include <apu/embedded_player.h>
#include <user/config.h>

#include <res/sounds/sys_worldmap_cursor.wav.h>
#include <res/sounds/sys_worldmap_finaldecide.wav.h>

enum EmbeddedSound
{
    EmbeddedSoundSysWorldMapCursor,
    EmbeddedSoundSysWorldMapFinalDecide,
    EmbeddedSoundCount,
};

static ma_engine g_audioEngine = {};
static bool g_audioEngineInitialized = false;
static std::array<ma_sound, EmbeddedSoundCount> g_embeddedSoundCache = {};
static std::array<ma_decoder, EmbeddedSoundCount> g_embeddedDecoderCache = {};
static const std::unordered_map<std::string, EmbeddedSound> g_embeddedSoundMap =
{
    { "sys_worldmap_cursor", EmbeddedSoundSysWorldMapCursor },
    { "sys_worldmap_finaldecide", EmbeddedSoundSysWorldMapFinalDecide },
};

void InitEmbeddedSound(EmbeddedSound s)
{
    ma_sound &sound = g_embeddedSoundCache[s];
    ma_decoder &decoder = g_embeddedDecoderCache[s];
    const void *soundData = nullptr;
    size_t soundDataSize = 0;
    float volume = Config::EffectsVolume;
    switch (s)
    {
    case EmbeddedSoundSysWorldMapCursor:
        soundData = g_sys_worldmap_cursor;
        soundDataSize = sizeof(g_sys_worldmap_cursor);
        volume *= 0.259f;
        break;
    case EmbeddedSoundSysWorldMapFinalDecide:
        soundData = g_sys_worldmap_finaldecide;
        soundDataSize = sizeof(g_sys_worldmap_finaldecide);
        volume *= 0.61f;
        break;
    default:
        assert(false && "Unknown embedded sound.");
        return;
    }

    if (soundData != nullptr)
    {
        ma_result res;
        res = ma_decoder_init_memory(soundData, soundDataSize, nullptr, &decoder);
        if (res != MA_SUCCESS)
        {
            fprintf(stderr, "ma_decoder_init_memory failed with error code %d on embedded sound %d.\n", res, s);
            return;
        }

        res = ma_sound_init_from_data_source(&g_audioEngine, &decoder, MA_SOUND_FLAG_DECODE, nullptr, &sound);
        if (res != MA_SUCCESS)
        {
            fprintf(stderr, "ma_sound_init_from_data_source failed with error code %d on embedded sound %d.\n", res, s);
            return;
        }

        ma_sound_set_volume(&sound, volume);
    }
}

void EmbeddedPlayer::Init() 
{
    s_isActive = true;
}

void EmbeddedPlayer::Play(const char *name) 
{
    assert(s_isActive && "Playback shouldn't be requested if the Embedded Player isn't active.");

    auto it = g_embeddedSoundMap.find(name);
    if (it == g_embeddedSoundMap.end())
    {
        return;
    }

    if (!g_audioEngineInitialized)
    {
        ma_engine_config engineConfig = ma_engine_config_init();
        ma_result res = ma_engine_init(&engineConfig, &g_audioEngine);
        if (res != MA_SUCCESS)
        {
            fprintf(stderr, "ma_engine_init failed with error code %d.\n", res);
        }

        // Do not attempt to initialize this again if it fails.
        g_audioEngineInitialized = true;
    }

    if (g_audioEngine.pDevice == nullptr)
    {
        return;
    }

    ma_sound &sound = g_embeddedSoundCache[it->second];
    if (sound.pDataSource == nullptr)
    {
        InitEmbeddedSound(it->second);
    }

    if (sound.pDataSource != nullptr)
    {
        ma_sound_seek_to_pcm_frame(&sound, 0);
        ma_sound_start(&sound);
    }
}

void EmbeddedPlayer::Shutdown() 
{
    for (ma_sound &sound : g_embeddedSoundCache)
    {
        if (sound.pDataSource != nullptr)
        {
            ma_sound_uninit(&sound);
        }
    }

    for (ma_decoder &decoder : g_embeddedDecoderCache)
    {
        if (decoder.pBackend != nullptr)
        {
            ma_decoder_uninit(&decoder);
        }
    }

    if (g_audioEngine.pDevice != nullptr)
    {
        ma_engine_uninit(&g_audioEngine);
    }

    g_audioEngineInitialized = false;
    s_isActive = false;
}
