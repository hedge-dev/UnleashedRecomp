#pragma once

class ISDLEventListener
{
public:
    virtual ~ISDLEventListener() = default;
    virtual void OnSDLEvent(SDL_Event* event) = 0;
};

extern std::vector<ISDLEventListener*>& GetEventListeners();

class SDLEventListener : public ISDLEventListener
{
public:
    SDLEventListener()
    {
        GetEventListeners().emplace_back(this);
    }

    void OnSDLEvent(SDL_Event* event) override {}
};
