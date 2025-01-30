#include <user/config.h>
#include <SWA/CharacterUtility/CharacterProxy.h>
#include <hid/hid.h>

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

// The barrel gets stuck at a slope at high frame rates and softlocks the player.
// We can update the character proxy at 30 FPS, and interpolate the visuals to work around this issue.
static constexpr size_t OBJ_BIG_BARREL_SIZE = 0x1A0;

struct ObjBigBarrelEx
{
    float elapsedTime{};
    bool interpolate{};
};

void ObjBigBarrelAllocMidAsmHook(PPCRegister& r3)
{
    r3.u32 += sizeof(ObjBigBarrelEx);
}

// CObjBigBarrel::CObjBigBarrel
PPC_FUNC_IMPL(__imp__sub_8271AC08);
PPC_FUNC(sub_8271AC08)
{
    new (base + ctx.r3.u32 + OBJ_BIG_BARREL_SIZE) ObjBigBarrelEx();
    __imp__sub_8271AC08(ctx, base);
}

// CObjBigBarrel::Integrate
PPC_FUNC_IMPL(__imp__sub_8271AA30);
PPC_FUNC(sub_8271AA30)
{
    auto objBigBarrelEx = reinterpret_cast<ObjBigBarrelEx*>(base + ctx.r3.u32 + OBJ_BIG_BARREL_SIZE);
    objBigBarrelEx->interpolate = ctx.f1.f64 < (1.0 / 30.0);
    objBigBarrelEx->elapsedTime += ctx.f1.f64;

    if (!objBigBarrelEx->interpolate || objBigBarrelEx->elapsedTime >= (1.0f / 30.0f))
    {
        ctx.f1.f64 = objBigBarrelEx->elapsedTime;
        __imp__sub_8271AA30(ctx, base);
        objBigBarrelEx->elapsedTime = 0.0f;
    }
}

void ObjBigBarrelSetPositionMidAsmHook(PPCRegister& r3, PPCRegister& r4)
{
    uint8_t* base = g_memory.base;
    auto objBigBarrelEx = reinterpret_cast<ObjBigBarrelEx*>(base + r3.u32 + OBJ_BIG_BARREL_SIZE);

    if (objBigBarrelEx->interpolate)
    {
        auto characterProxy = reinterpret_cast<SWA::CCharacterProxy*>(base + PPC_LOAD_U32(r3.u32 + 0x100));
        auto position = reinterpret_cast<Hedgehog::Math::CVector*>(base + r4.u32);

        float factor = (1.0f / 30.0f) - objBigBarrelEx->elapsedTime;
        position->X = position->X - characterProxy->m_Velocity.X * factor;
        position->Y = position->Y - characterProxy->m_Velocity.Y * factor;
        position->Z = position->Z - characterProxy->m_Velocity.Z * factor;
    }
}

// Tornado Defense bullet particles are colored by the button prompt, which differs on PlayStation 3.
// Luckily, the PS3 particles are left in the files, and they get spawned by name when a bullet gets created.

// SWA::CExBullet::AddCallback
PPC_FUNC_IMPL(__imp__sub_82B14CC0);
PPC_FUNC(sub_82B14CC0)
{
    auto isPlayStation = Config::ControllerIcons == EControllerIcons::PlayStation;

    if (Config::ControllerIcons == EControllerIcons::Auto)
        isPlayStation = hid::g_inputDeviceController == hid::EInputDevice::PlayStation;

    if (isPlayStation)
    {
        PPC_STORE_U8(0x820C2A0B, 'b'); // Cross
        PPC_STORE_U8(0x820C29C3, 'r'); // Circle
        PPC_STORE_U8(0x820C29DB, 'p'); // Square
        PPC_STORE_U8(0x820C29F3, 'g'); // Triangle
    }
    else
    {
        PPC_STORE_U8(0x820C2A0B, 'g'); // A
        PPC_STORE_U8(0x820C29C3, 'r'); // B
        PPC_STORE_U8(0x820C29DB, 'b'); // X
        PPC_STORE_U8(0x820C29F3, 'o'); // Y
    }

    __imp__sub_82B14CC0(ctx, base);
}
