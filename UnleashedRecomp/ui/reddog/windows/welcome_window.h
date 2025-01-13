#pragma once

#include <ui/game_window.h>
#include <ui/reddog/reddog_window.h>

class WelcomeWindow : public Reddog::Window
{
public:
    WelcomeWindow() : Window()
    {
        Name = "Welcome";
        Flags = (Reddog::EWindowFlags)(Reddog::eWindowFlags_NoResize | Reddog::eWindowFlags_NoListEntry | Reddog::eWindowFlags_Centre);
        IsMinimumVisible = false;
        IsPinVisible = false;
        IsPinned = false;
        IsVisible = true;
    }

    void Draw() override;
};
