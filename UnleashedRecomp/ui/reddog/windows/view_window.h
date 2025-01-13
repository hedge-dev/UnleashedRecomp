#pragma once

#include <ui/reddog/reddog_window.h>

class ViewWindow : public Reddog::Window
{
public:
    ViewWindow() : Window()
    {
        Name = "View";
        Flags = Reddog::eWindowFlags_NoResize;
    }

    void Draw() override;
};
