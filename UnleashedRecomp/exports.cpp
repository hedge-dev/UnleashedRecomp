#include <apu/embedded_player.h>
#include <kernel/function.h>
#include <kernel/heap.h>
#include <kernel/memory.h>
#include <cpu/guest_stack_var.h>
#include <ui/installer_wizard.h>
#include <ui/game_window.h>
#include <api/boost/smart_ptr/shared_ptr.h>

SWA_API void Game_PlaySound(const char* pName)
{
    if (EmbeddedPlayer::s_isActive)
    {
        EmbeddedPlayer::Play(pName);
    }
    else
    {
        guest_stack_var<boost::anonymous_shared_ptr> soundPlayer;
        GuestToHostFunction<void>(sub_82B4DF50, soundPlayer.get(), ((be<uint32_t>*)g_memory.Translate(0x83367900))->get(), 7, 0, 0);

        auto soundPlayerVtable = (be<uint32_t>*)g_memory.Translate(*(be<uint32_t>*)soundPlayer->get());
        uint32_t virtualFunction = *(soundPlayerVtable + 1);

        size_t strLen = strlen(pName);
        void *strAllocation = g_userHeap.Alloc(strLen + 1);
        memcpy(strAllocation, pName, strLen + 1);
        GuestToHostFunction<void>(virtualFunction, soundPlayer->get(), strAllocation, 0);
        g_userHeap.Free(strAllocation);
    }
}

SWA_API void Window_SetDisplay(int displayIndex)
{
    GameWindow::SetDisplay(displayIndex);
}

SWA_API void Window_SetFullscreen(bool isEnabled)
{
    GameWindow::SetFullscreen(isEnabled);
}
