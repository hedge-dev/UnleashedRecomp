#include <cpu/guest_code.h>
#include <cfg/config.h>

PPC_FUNC_IMPL(__imp__sub_82B4E130);
PPC_FUNC(sub_82B4E130)
{
    auto pMusicVolume = (be<float>*)g_memory.Translate(ctx.r3.u32);

    *pMusicVolume = Config::MusicVolume;

    __imp__sub_82B4E130(ctx, base);
}
