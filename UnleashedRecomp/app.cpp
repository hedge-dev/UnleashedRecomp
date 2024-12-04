#include <app.h>
#include <kernel/function.h>
#include <ui/window.h>
#include <patches/audio_patches.h>

bool g_isGameLoaded = false;
double g_deltaTime;

// CApplication::Update
PPC_FUNC_IMPL(__imp__sub_822C1130);
PPC_FUNC(sub_822C1130)
{
    g_isGameLoaded = true;
    g_deltaTime = ctx.f1.f64;

    SDL_PumpEvents();
    SDL_FlushEvents(SDL_FIRSTEVENT, SDL_LASTEVENT);

    Window::Update();
    AudioPatches::Update(g_deltaTime);

    __imp__sub_822C1130(ctx, base);
}
