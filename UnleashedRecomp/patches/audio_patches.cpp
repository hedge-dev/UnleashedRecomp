#include <cpu/guest_code.h>
#include <user/config.h>
#include <kernel/function.h>
#include <kernel/platform.h>
#include <patches/audio_patches.h>
#include <api/SWA.h>

#if _WIN32
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Media.Control.h>

using namespace winrt;
using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::Media::Control;

GlobalSystemMediaTransportControlsSessionManager m_sessionManager = nullptr;

GlobalSystemMediaTransportControlsSessionManager GetSessionManager()
{
    if (m_sessionManager)
        return m_sessionManager;

    init_apartment();

    return m_sessionManager = GlobalSystemMediaTransportControlsSessionManager::RequestAsync().get();
}

GlobalSystemMediaTransportControlsSession GetCurrentSession()
{
    return GetSessionManager().GetCurrentSession();
}

bool IsExternalAudioPlaying()
{
    auto session = GetCurrentSession();

    if (!session)
        return false;

    return session.GetPlaybackInfo().PlaybackStatus() == GlobalSystemMediaTransportControlsSessionPlaybackStatus::Playing;
}

int AudioPatches::m_isAttenuationSupported = -1;
#endif

be<float>* GetVolume(bool isMusic = true)
{
    auto ppUnkClass = (be<uint32_t>*)g_memory.Translate(0x83362FFC);

    if (!ppUnkClass->get())
        return nullptr;

    // NOTE (Hyper): This is fine, trust me. See 0x82E58728.
    return (be<float>*)g_memory.Translate(4 * ((int)isMusic + 0x1C) + ((be<uint32_t>*)g_memory.Translate(ppUnkClass->get() + 4))->get());
}

bool AudioPatches::CanAttenuate()
{
#if _WIN32
    if (m_isAttenuationSupported >= 0)
        return m_isAttenuationSupported;

    auto version = GetPlatformVersion();

    m_isAttenuationSupported = version.Major >= 10 && version.Build >= 17763;

    return m_isAttenuationSupported;
#else
    return false;
#endif
}

void AudioPatches::Update(float deltaTime)
{
    auto pMusicVolume = GetVolume();
    auto pEffectsVolume = GetVolume(false);

    if (!pMusicVolume || !pEffectsVolume)
        return;

#if _WIN32
    if (Config::MusicAttenuation && CanAttenuate())
    {
        auto time = 1.0f - expf(2.5f * -deltaTime);

        if (IsExternalAudioPlaying())
        {
            *pMusicVolume = std::lerp(*pMusicVolume, 0.0f, time);
        }
        else
        {
            *pMusicVolume = std::lerp(*pMusicVolume, Config::MusicVolume, time);
        }
    }
    else
#endif
    {
        *pMusicVolume = Config::MusicVolume;
    }

    *pEffectsVolume = Config::EffectsVolume;
}

// Stub volume setter.
GUEST_FUNCTION_STUB(sub_82E58728);
