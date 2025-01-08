#include "window_list.h"
#include <ui/reddog/reddog_controls.h>

static WindowList g_window;

void WindowList::Draw()
{
    if (Begin())
    {
        for (auto& pWindow : Reddog::GetWindows())
        {
            auto pTrueWindow = (Reddog::Window*)pWindow;

            if ((pTrueWindow->Flags & Reddog::eWindowFlags_NoListEntry) != 0)
                continue;

            if (Reddog::ExplicitButton(pTrueWindow->Name, Reddog::EButtonTextAlignment::Left, { 190, 32 }, 1.1f))
                pTrueWindow->SetVisible();
        }
    }
    End();
}
