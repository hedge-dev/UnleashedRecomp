#include <api/SWA.h>
#include <ui/game_window.h>
#include <user/config.h>
#include <ui/reddog/debug_draw.h>
#include <ui/imgui_utils.h>
#include <patches/aspect_ratio_patches.h>


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

    __imp__sub_82522040(ctx, base);

    // Draw player position
    if (Reddog::DebugDraw::GetIsDrawPosition())
    {
        // TODO (RadiantDerg): Reimplement SWA::CStageManager ability to draw progress ratio
        // NOTE: Currently does not work, is the API mapping horribly misaligned/misunderstood?
        if (a1->m_spStageGuidePathController)
        {
            if (a1->m_spStageGuidePathController->m_spPathAnimationController)
            {
                auto PAC = a1->m_spStageGuidePathController->m_spPathAnimationController;
                //Reddog::DebugDraw::DrawTextLog(fmt::format("m_DistanceAlongPath = {:.2f}", PAC->m_DistanceAlongPath.get()).c_str());
            }

            // Ratio
            if (a1->m_StageGuidePathLength.get() > 0.0f)
            {
                //float distance = 0; // GetDistanceAlongPath(a1->m_StageGuidePathController)
                //a1->m_StageGuidePathRatio = distance / a1->m_StageGuidePathLength;

                const Reddog::SDrawText ratioText{
                {Scale(g_aspectRatioOffsetX + 720), Scale(g_aspectRatioOffsetY + 36)},
                    fmt::format("{:.1f}m [{:.1f}/100.0]", a1->m_StageGuidePathLength.get(), a1->m_StageGuidePathRatio.get()),
                    0,
                    3.25f,
                    0xFFFFFFFF,
                    Reddog::eDrawTextFlags_NoShadow
                };

                Reddog::DebugDraw::DrawText2D(ratioText);
            }
        }

        // Position
        const Reddog::SDrawText positionText{
            {Scale(g_aspectRatioOffsetX + 750), Scale(g_aspectRatioOffsetY + 120)},
            fmt::format("( {:.2f}, {:.2f}, {:.2f} )", a1->m_PlayerPosition.X.get(), a1->m_PlayerPosition.Y.get(), a1->m_PlayerPosition.Z.get()),
            0,
            2.0f,
            0xFFFFFFFF,
            Reddog::eDrawTextFlags_NoShadow
        };

        Reddog::DebugDraw::DrawText2D(positionText);
    }

}

// GetIsDebugRenderForGameObject()
//PPC_FUNC(sub_82512BF8)
//{
//    ctx.r3.u8 = 1; // Always return true
//}
