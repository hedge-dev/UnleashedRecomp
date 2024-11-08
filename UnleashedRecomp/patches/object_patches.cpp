#include <cpu/guest_code.h>

// CObjFlame::CObjFlame
// A field is not zero initialized,
// causing collisions to constantly get created
// and slow down the game.
PPC_FUNC_IMPL(__imp__sub_82608E60);
PPC_FUNC(sub_82608E60)
{
    memset(base + ctx.r3.u32, 0, 0x154);
    __imp__sub_82608E60(ctx, base);
}
