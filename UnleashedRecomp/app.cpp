#include <app.h>
#include <install/installer.h>
#include <kernel/function.h>
#include <ui/game_window.h>
#include <patches/audio_patches.h>
#include <user/config.h>
#include <os/process.h>

void FrameLimiter::execute(int64_t fps)
{
    using namespace std::chrono_literals;

    auto now = std::chrono::steady_clock::now();

    if (now < next)
    {
        std::this_thread::sleep_for(std::chrono::floor<std::chrono::milliseconds>(next - now - 1ms));

        while ((now = std::chrono::steady_clock::now()) < next)
            std::this_thread::yield();
    }
    else
    {
        next = now;
    }

    next += 1000000000ns / fps;
}

static FrameLimiter g_frameLimiter;

void ApplicationUpdateMidAsmHook()
{
    if (Config::FPS >= 15 && Config::FPS < 240)
        g_frameLimiter.execute(Config::FPS);
}

void App::Restart(std::vector<std::string> restartArgs)
{
    os::process::StartProcess(os::process::GetExecutablePath(), restartArgs, os::process::GetWorkingDirectory());
    Exit();
}

void App::Exit()
{
    Config::Save();

#ifdef _WIN32
    timeEndPeriod(1);
#endif

    std::_Exit(0);
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

    GameWindow::Update();
    AudioPatches::Update(App::s_deltaTime);

    __imp__sub_822C1130(ctx, base);
}

