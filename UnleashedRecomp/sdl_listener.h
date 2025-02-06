#pragma once

class ISDLEventListener
{
public:
    virtual ~ISDLEventListener() = default;
    virtual bool OnSDLEvent(SDL_Event* event) = 0;
};

extern std::vector<ISDLEventListener*>& GetEventListeners();

class SDLEventListener : public ISDLEventListener
{
public:
    SDLEventListener()
    {
        GetEventListeners().emplace_back(this);
    }

    bool OnSDLEvent(SDL_Event* event) override { return false; }
};
