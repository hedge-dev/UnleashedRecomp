#pragma once

#include <ui/reddog/reddog_window.h>

class ExportsWindow : public Reddog::Window
{
public:
    using Reddog::Window::Window;

    void Draw() override;
};
