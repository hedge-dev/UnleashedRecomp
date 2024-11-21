#include <cpu/guest_code.h>
#include <cfg/config.h>
#include <kernel/function.h>
#include <patches/audio_patches.h>

be<float>* GetVolume(bool isMusic = true)
{
    auto ppUnkClass = (be<uint32_t>*)g_memory.Translate(0x83362FFC);

    if (!ppUnkClass->get())
        return nullptr;

    // NOTE (Hyper): This is fine, trust me. See 0x82E58728.
    return (be<float>*)g_memory.Translate(4 * ((int)isMusic + 0x1C) + ((be<uint32_t>*)g_memory.Translate(ppUnkClass->get() + 4))->get());
}

void AudioPatches::Update(float deltaTime)
{
    auto pMusicVolume = GetVolume();
    auto pEffectsVolume = GetVolume(false);

    if (!pMusicVolume || !pEffectsVolume)
        return;

    *pMusicVolume = Config::MusicVolume;
    *pEffectsVolume = Config::EffectsVolume;
}

// Stub volume setter.
GUEST_FUNCTION_STUB(sub_82E58728);
