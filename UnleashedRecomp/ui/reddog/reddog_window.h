#pragma once

#define REDDOG_IMGUI_FLAGS ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoBackground

namespace Reddog
{
    enum EWindowFlags : uint32_t
    {
        eWindowFlags_None         = 0,
        eWindowFlags_NoTitleBar   = 1 << 0,
        eWindowFlags_NoResize     = 1 << 1,
        eWindowFlags_NoListEntry  = 1 << 2,
        eWindowFlags_Centre       = 1 << 3
    };

    class IWindow
    {
    public:
        virtual ~IWindow() = default;
        virtual const char* GetName() = 0;
        virtual void Draw() = 0;
    };

    extern std::vector<IWindow*>& GetWindows();

    class Window : public IWindow
    {
    private:
        bool* m_pIsVisible{ nullptr };

        bool m_isSetCentre{};
        bool m_isSetVisible{};

        bool m_isCloseButtonPressed{};
        bool m_isMinimumButtonPressed{};
        bool m_isPinButtonPressed{};

        ImVec2 m_preMinimumWindowSize{};

        void BeginStyle();
        void UpdateStyle();
        void EndStyle();
        void CreateWindowGrips();
        void DrawFrame();

    protected:
        float m_minWidth{ 175.0f };
        float m_minHeight{};
        float m_maxWidth{ FLT_MAX };
        float m_maxHeight{ FLT_MAX };

        std::function<void()> m_preBegin{ nullptr };
        std::function<void()> m_postBegin{ nullptr };
        std::function<void()> m_preEnd{ nullptr };
        std::function<void()> m_postEnd{ nullptr };

    public:
        const char* Name{ "(null)" };
        EWindowFlags Flags{ eWindowFlags_None };
        bool IsFocused{ true };
        bool IsCloseVisible{ true };
        bool IsMinimumVisible{ true };
        bool IsPinVisible{ true };
        bool IsMinimum{};
        bool IsPinned{ true };
        bool IsVisible{};
        ImGuiWindow* ImWindow{ nullptr };
        ImU32 ActiveColour{ IM_COL32(227, 227, 255, 255) };
        ImU32 InactiveColour{ IM_COL32(182, 182, 182, 255) };

        Window()
        {
            GetWindows().push_back(this);
        }

        Window(const char* pName, EWindowFlags flags = eWindowFlags_None) : Name(pName), Flags(flags)
        {
            GetWindows().push_back(this);
        }

        const char* GetName() override
        {
            return Name;
        }

        void Draw() override
        {
            Begin();
            End();
        }

        void SetMinimum(bool isMinimum);
        void SetVisible(bool isVisible = true);
        bool Begin(bool* pIsVisible = nullptr);
        void End();
    };

    void InitWindowResources();
}
