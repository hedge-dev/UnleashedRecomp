#pragma once

#include <ui/reddog/reddog_window.h>

class ExportsWindow : public Reddog::Window
{
public:
    ExportsWindow() : Window()
    {
        Name = "Exports";
        Flags = Reddog::eWindowFlags_NoResize;
    }

    void Draw() override;
};
