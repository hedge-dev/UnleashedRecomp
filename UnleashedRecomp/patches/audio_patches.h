#pragma once

class AudioPatches
{
protected:
    static int m_isAttenuationSupported;

public:
    static bool CanAttenuate();
    static void Update(float deltaTime);
};
