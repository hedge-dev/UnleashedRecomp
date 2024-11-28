#include "imgui_view.h"

std::vector<IImGuiView*>& GetImGuiViews()
{
    static std::vector<IImGuiView*> g_imGuiViews;
    return g_imGuiViews;
}
