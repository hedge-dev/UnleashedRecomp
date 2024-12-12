#include <app.h>
#include <install/installer.h>
#include <kernel/function.h>
#include <ui/window.h>
#include <patches/audio_patches.h>
#include <user/config.h>
#include <os/process.h>

void App::Restart(std::vector<std::string> restartArgs)
{
    os::process::StartProcess(os::process::GetExecutablePath(), restartArgs, os::process::GetWorkingDirectory());
    Exit();
}

void App::Exit()
{
    Config::Save();

#if _WIN32
    ExitProcess(0);
#endif
}

// CApplication::Ctor
PPC_FUNC_IMPL(__imp__sub_824EB490);
PPC_FUNC(sub_824EB490)
{
    App::s_isInit = true;
    App::s_isMissingDLC = !Installer::checkAllDLC(GetGamePath());
    App::s_language = Config::Language;

    __imp__sub_824EB490(ctx, base);
}

// CApplication::Update
PPC_FUNC_IMPL(__imp__sub_822C1130);
PPC_FUNC(sub_822C1130)
{
    App::s_deltaTime = ctx.f1.f64;

    SDL_PumpEvents();
    SDL_FlushEvents(SDL_FIRSTEVENT, SDL_LASTEVENT);

    Window::Update();
    AudioPatches::Update(App::s_deltaTime);

    __imp__sub_822C1130(ctx, base);
}
