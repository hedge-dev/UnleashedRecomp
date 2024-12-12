#include <miniaudio.h>

#include <apu/embedded_player.h>
#include <user/config.h>

#include <res/sounds/sys_worldmap_cursor.wav.h>
#include <res/sounds/sys_worldmap_finaldecide.wav.h>
#include <res/sounds/sys_actstg_pausecansel.wav.h>
#include <res/sounds/sys_actstg_pausecursor.wav.h>
#include <res/sounds/sys_actstg_pausedecide.wav.h>
#include <res/sounds/sys_actstg_pausewinclose.wav.h>
#include <res/sounds/sys_actstg_pausewinopen.wav.h>

enum class EmbeddedSound
{
    SysWorldMapCursor,
    SysWorldMapFinalDecide,
    SysActStgPauseCansel,
    SysActStgPauseCursor,
    SysActStgPauseDecide,
    SysActStgPauseWinClose,
    SysActStgPauseWinOpen,
    Count,
};

struct EmbeddedSoundData
{
    static const int SimultaneousLimit = 4;
    std::array<std::unique_ptr<ma_sound>, SimultaneousLimit> sounds;
    std::array<std::unique_ptr<ma_decoder>, SimultaneousLimit> decoders;
    int oldestIndex = 0;
};

static ma_engine g_audioEngine = {};
static bool g_audioEngineInitialized = false;
static std::array<EmbeddedSoundData, size_t(EmbeddedSound::Count)> g_embeddedSoundData = {};
static const std::unordered_map<std::string, EmbeddedSound> g_embeddedSoundMap =
{
    { "sys_worldmap_cursor", EmbeddedSound::SysWorldMapCursor },
    { "sys_worldmap_finaldecide", EmbeddedSound::SysWorldMapFinalDecide },
    { "sys_actstg_pausecansel", EmbeddedSound::SysActStgPauseCansel },
    { "sys_actstg_pausecursor", EmbeddedSound::SysActStgPauseCursor },
    { "sys_actstg_pausedecide", EmbeddedSound::SysActStgPauseDecide },
    { "sys_actstg_pausewinclose", EmbeddedSound::SysActStgPauseWinClose },
    { "sys_actstg_pausewinopen", EmbeddedSound::SysActStgPauseWinOpen },
};

static void PlayEmbeddedSound(EmbeddedSound s)
{
    EmbeddedSoundData &data = g_embeddedSoundData[size_t(s)];
    int pickedIndex = -1;
    for (int i = 0; (i < EmbeddedSoundData::SimultaneousLimit) && (pickedIndex < 0); i++)
    {
        if (data.sounds[i] == nullptr)
        {
            // The sound hasn't been created yet, create it and pick it.
            const void *soundData = nullptr;
            size_t soundDataSize = 0;
            switch (s)
            {
            case EmbeddedSound::SysWorldMapCursor:
                soundData = g_sys_worldmap_cursor;
                soundDataSize = sizeof(g_sys_worldmap_cursor);
                break;
            case EmbeddedSound::SysWorldMapFinalDecide:
                soundData = g_sys_worldmap_finaldecide;
                soundDataSize = sizeof(g_sys_worldmap_finaldecide);
                break;
            case EmbeddedSound::SysActStgPauseCansel:
                soundData = g_sys_actstg_pausecansel;
                soundDataSize = sizeof(g_sys_actstg_pausecansel);
                break;
            case EmbeddedSound::SysActStgPauseCursor:
                soundData = g_sys_actstg_pausecursor;
                soundDataSize = sizeof(g_sys_actstg_pausecursor);
                break;
            case EmbeddedSound::SysActStgPauseDecide:
                soundData = g_sys_actstg_pausedecide;
                soundDataSize = sizeof(g_sys_actstg_pausedecide);
                break;
            case EmbeddedSound::SysActStgPauseWinClose:
                soundData = g_sys_actstg_pausewinclose;
                soundDataSize = sizeof(g_sys_actstg_pausewinclose);
                break;
            case EmbeddedSound::SysActStgPauseWinOpen:
                soundData = g_sys_actstg_pausewinopen;
                soundDataSize = sizeof(g_sys_actstg_pausewinopen);
                break;
            default:
                assert(false && "Unknown embedded sound.");
                return;
            }

            ma_result res;
            data.decoders[i] = std::make_unique<ma_decoder>();
            res = ma_decoder_init_memory(soundData, soundDataSize, nullptr, data.decoders[i].get());
            if (res != MA_SUCCESS)
            {
                fprintf(stderr, "ma_decoder_init_memory failed with error code %d.\n", res);
                return;
            }

            data.sounds[i] = std::make_unique<ma_sound>();
            res = ma_sound_init_from_data_source(&g_audioEngine, data.decoders[i].get(), MA_SOUND_FLAG_DECODE, nullptr, data.sounds[i].get());
            if (res != MA_SUCCESS)
            {
                fprintf(stderr, "ma_sound_init_from_data_source failed with error code %d.\n", res);
                return;
            }

            pickedIndex = i;
        }
        else if (ma_sound_at_end(data.sounds[i].get()))
        {
            // A sound has reached the end, pick it.
            pickedIndex = i;
        }
    }

    if (pickedIndex < 0)
    {
        // No free slots are available, pick the oldest one.
        pickedIndex = data.oldestIndex;
        data.oldestIndex = (data.oldestIndex + 1) % EmbeddedSoundData::SimultaneousLimit;
    }

    if (data.sounds[pickedIndex] != nullptr)
    {
        ma_sound_set_volume(data.sounds[pickedIndex].get(), Config::EffectsVolume);
        ma_sound_seek_to_pcm_frame(data.sounds[pickedIndex].get(), 0);
        ma_sound_start(data.sounds[pickedIndex].get());
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

    PlayEmbeddedSound(it->second);
}

void EmbeddedPlayer::Shutdown() 
{
    for (EmbeddedSoundData &data : g_embeddedSoundData)
    {
        for (auto &sound : data.sounds)
        {
            if (sound != nullptr)
            {
                if (sound->pDataSource != nullptr)
                {
                    ma_sound_uninit(sound.get());
                }

                sound.reset();
            }
        }

        for (auto &decoder : data.decoders)
        {
            if (decoder != nullptr)
            {
                if (decoder->pBackend != nullptr)
                {
                    ma_decoder_uninit(decoder.get());
                }

                decoder.reset();
            }
        }
    }

    if (g_audioEngine.pDevice != nullptr)
    {
        ma_engine_uninit(&g_audioEngine);
    }

    g_audioEngineInitialized = false;
    s_isActive = false;
}
