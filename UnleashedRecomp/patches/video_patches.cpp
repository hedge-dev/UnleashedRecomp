#include <user/config.h>
#include <api/SWA.h>
#include <ui/game_window.h>

// Update movie player aspect ratio.
PPC_FUNC_IMPL(__imp__sub_82AE30D8);
PPC_FUNC(sub_82AE30D8)
{
    auto movieWidth = *(be<float>*)g_memory.Translate(ctx.r4.u32 - 0x10);
    auto movieHeight = *(be<float>*)g_memory.Translate(ctx.r4.u32 - 0x0C);
    auto movieAspectRatio = movieWidth / movieHeight;
    auto windowAspectRatio = (float)GameWindow::s_width / (float)GameWindow::s_height;

    auto pTopLeft = (Vertex*)g_memory.Translate(ctx.r4.u32 + 0x6C);
    auto pTopRight = (Vertex*)g_memory.Translate(ctx.r4.u32 + 0x6C + sizeof(Vertex));
    auto pBottomRight = (Vertex*)g_memory.Translate(ctx.r4.u32 + 0x6C + sizeof(Vertex) * 2);
    auto pBottomLeft = (Vertex*)g_memory.Translate(ctx.r4.u32 + 0x6C + sizeof(Vertex) * 3);

    auto a = -1.00078f;
    auto b = 1.00139f;
    auto scaleU = 1.0f;
    auto scaleV = 1.0f;
    auto centreV = (pTopLeft->V + pBottomRight->V) / 2.0f;

    if (windowAspectRatio > movieAspectRatio)
    {
        scaleU = movieAspectRatio / windowAspectRatio;
    }
    else
    {
        scaleV = windowAspectRatio / movieAspectRatio;
    }

    pTopLeft->X = a;
    pTopLeft->Y = b;
    pTopLeft->U = (pTopLeft->U - centreV) / scaleU + centreV;
    pTopLeft->V = (pTopLeft->V - centreV) / scaleV + centreV;

    pTopRight->X = b;
    pTopRight->Y = b;
    pTopRight->U = (pTopRight->U - centreV) / scaleU + centreV;
    pTopRight->V = (pTopRight->V - centreV) / scaleV + centreV;

    pBottomLeft->X = a;
    pBottomLeft->Y = a;
    pBottomLeft->U = (pBottomLeft->U - centreV) / scaleU + centreV;
    pBottomLeft->V = (pBottomLeft->V - centreV) / scaleV + centreV;

    pBottomRight->X = b;
    pBottomRight->Y = a;
    pBottomRight->U = (pBottomRight->U - centreV) / scaleU + centreV;
    pBottomRight->V = (pBottomRight->V - centreV) / scaleV + centreV;

    __imp__sub_82AE30D8(ctx, base);
}

void RemoveMoviePlayerLetterboxMidAsmHook() {}

bool MotionBlurMidAsmHook()
{
    return Config::MotionBlur != EMotionBlur::Off;
}
