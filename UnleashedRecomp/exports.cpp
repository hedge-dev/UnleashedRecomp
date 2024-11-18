#include <ui/window.h>

extern "C" void Window_SetFullscreen(bool isEnabled)
{
    Window::SetFullscreen(isEnabled);
}
