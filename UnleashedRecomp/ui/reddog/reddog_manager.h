#pragma once

#include <ui/reddog/reddog_window.h>

namespace Reddog
{
    class Manager
    {
    public:
        static inline bool s_isVisible = false;

        static void Init();
        static void Draw();

        static Window* GetWindow(const char* pName);

        template<typename T>
        static T* GetWindow(const char* pName);
    };
}
