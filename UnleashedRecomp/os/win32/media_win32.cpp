#include <os/media.h>
#include <os/logger.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Media.Control.h>

using namespace winrt;
using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::Media::Control;

static GlobalSystemMediaTransportControlsSessionManager g_sessionManager = nullptr;

static GlobalSystemMediaTransportControlsSessionManager GetSessionManager()
{
    if (g_sessionManager)
        return g_sessionManager;

    try
    {
        init_apartment();
        return g_sessionManager = GlobalSystemMediaTransportControlsSessionManager::RequestAsync().get();
    }
    catch (...)
    {
        LOGF_ERROR("Failed to retrieve GSMTC session manager: 0x{:X}", to_hresult().value);
        return nullptr;
    }
}

static GlobalSystemMediaTransportControlsSession GetCurrentSession()
{
    auto sessionManager = GetSessionManager();

    if (!sessionManager)
        return nullptr;

    try
    {
        return sessionManager.GetCurrentSession();
    }
    catch (...)
    {
        LOGF_ERROR("Failed to retrieve current GSMTC session: 0x{:X}", to_hresult().value);
        return nullptr;
    }
}

static GlobalSystemMediaTransportControlsSessionPlaybackInfo GetPlaybackInfo()
{
    auto session = GetCurrentSession();

    if (!session)
        return nullptr;

    try
    {
        return session.GetPlaybackInfo();
    }
    catch (...)
    {
        LOGF_ERROR("Failed to retrieve GSMTC playback info: 0x{:X}", to_hresult().value);
        return nullptr;
    }
}

bool os::media::IsExternalMediaPlaying()
{
    auto playbackInfo = GetPlaybackInfo();

    if (!playbackInfo)
        return false;

    try
    {
        return playbackInfo.PlaybackStatus() == GlobalSystemMediaTransportControlsSessionPlaybackStatus::Playing;
    }
    catch (...)
    {
        LOGF_ERROR("Failed to retrieve GSMTC playback status: 0x{:X}", to_hresult().value);
        return false;
    }
}
