#pragma once

#include <ui/reddog/reddog_window.h>

#define WINDOW_LIST_NAME "##WindowList"

class WindowList : public Reddog::Window
{
public:
    WindowList() : Window()
    {
        Name = WINDOW_LIST_NAME;
        Flags = (Reddog::EWindowFlags)(Reddog::eWindowFlags_NoTitleBar | Reddog::eWindowFlags_NoResize | Reddog::eWindowFlags_NoListEntry);
        ActiveColour = InactiveColour = IM_COL32_WHITE;

        m_minWidth = m_minHeight = 0;
    }

    void Draw() override;
};
