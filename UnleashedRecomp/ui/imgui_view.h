#pragma once

class IImGuiView
{
public:
    virtual ~IImGuiView() = default;
    virtual void Init() = 0;
    virtual void Draw() = 0;
};

std::vector<IImGuiView*>& GetImGuiViews();

class ImGuiView : public IImGuiView
{
public:
    ImGuiView()
    {
        GetImGuiViews().emplace_back(this);
    }

    void Init() override {}
    void Draw() override {}
};
