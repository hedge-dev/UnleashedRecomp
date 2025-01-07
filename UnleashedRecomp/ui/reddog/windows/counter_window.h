#pragma once

#include <ui/reddog/reddog_window.h>

class CounterWindow : public Reddog::Window
{
public:
    CounterWindow() : Window()
    {
        Name = "Counter";

        m_minWidth = 215;
        m_minHeight = 328;
    }

    void Draw() override;
};
