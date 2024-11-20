#include <kernel/function.h>
#include <kernel/heap.h>
#include <kernel/memory.h>
#include <ui/window.h>

extern "C" void Game_PlaySound(const char* pName)
{
    void* soundPlayerSharedPtr = g_userHeap.Alloc(8);
    GuestToHostFunction<void>(0x82B4DF50, soundPlayerSharedPtr, ((be<uint32_t>*)g_memory.Translate(0x83367900))->get(), 7, 0, 0);

    auto soundPlayer = (be<uint32_t>*)g_memory.Translate(*(be<uint32_t>*)soundPlayerSharedPtr);
    auto soundPlayerVtable = (be<uint32_t>*)g_memory.Translate(*soundPlayer);
    uint32_t virtualFunction = *(soundPlayerVtable + 1);

    size_t strLen = strlen(pName);
    void* strAllocation = g_userHeap.Alloc(strLen + 1);
    memcpy(strAllocation, pName, strLen + 1);
    GuestToHostFunction<void>(virtualFunction, soundPlayer, strAllocation, 0);
    g_userHeap.Free(strAllocation);

    GuestToHostFunction<void>(0x822C0890, *((be<uint32_t>*)soundPlayerSharedPtr + 1));
    g_userHeap.Free(soundPlayerSharedPtr);
}

extern "C" void Window_SetFullscreen(bool isEnabled)
{
    Window::SetFullscreen(isEnabled);
}
