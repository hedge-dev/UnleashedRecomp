#include <api/SWA.h>
#include <ui/game_window.h>
#include <user/config.h>
#include <ui/reddog/debug_draw.h>
#include <ui/imgui_utils.h>


// boost::~::SWA::CDebugDraw::CMember::SDrawLine
PPC_FUNC_IMPL(__imp__sub_822C9398);
PPC_FUNC(sub_822C9398)
{
    auto a2 = (Hedgehog::Math::CVector*)g_memory.Translate(ctx.r4.u32);
    auto a3 = (Hedgehog::Math::CVector*)g_memory.Translate(ctx.r5.u32);
    auto a4 = (be<unsigned int>*)g_memory.Translate(ctx.r6.u32);

    Reddog::Vector3 start(a2->X, a2->Y, a2->Z);
    Reddog::Vector3 end(a3->X, a3->Y, a3->Z);

    const Reddog::SDrawLine line{
        start, end, a4->value
    };

    Reddog::DebugDraw::DrawLine(line);

    __imp__sub_822C9398(ctx, base);
}


// SWA::CStageManager::UpdateParallel
//PPC_FUNC_IMPL(__imp__sub_82521C68);
//PPC_FUNC(sub_82521C68)
//{
//    __imp__sub_82521C68(ctx, base);
//}


// SWA::CStageManager::UpdateSerial
PPC_FUNC_IMPL(__imp__sub_82522040);
PPC_FUNC(sub_82522040)
{
    auto a1 = static_cast<SWA::CStageManager*>(g_memory.Translate(ctx.r3.u32));

    // Draw player position
    if (Reddog::DebugDraw::GetIsDrawPosition())
    {
        const Reddog::SDrawText positionText{
        {Scale(750), Scale(120)},
        fmt::format("( {:.2f}, {:.2f}, {:.2f} )", a1->m_PlayerPosition.X.get(), a1->m_PlayerPosition.Y.get(), a1->m_PlayerPosition.Z.get()),
        0,
        Scale(1.0f),
        0xFFFFFFFF,
        Reddog::eDrawTextFlags_NoShadow
        };

        Reddog::DebugDraw::DrawText2D(positionText);
    }
	
	// TODO (RadiantDerg): Reimplement SWA::CStageManager ability to draw progress ratio

    __imp__sub_82522040(ctx, base);
}

// GetIsDebugRenderForGameObject()
PPC_FUNC(sub_82512BF8)
{
    ctx.r3.u8 = 1; // Always return true
}
