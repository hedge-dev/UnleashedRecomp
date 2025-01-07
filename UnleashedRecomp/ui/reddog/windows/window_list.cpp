#include "window_list.h"

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

            if (ImGui::Button(pTrueWindow->Name, { 190, 26 }))
                pTrueWindow->SetVisible();
        }
    }
    End();
}
