#pragma once

// On-screen touch gamepad, modelled after the default XeniOS layout:
// a movement stick with a D-Pad ring on the left, swipe-to-look anywhere on
// the screen that isn't a control, and face/shoulder/trigger buttons on the right.
namespace TouchControls
{
    void Init();

    // Draws the overlay. Coordinates are mapped from the window to the ImGui
    // viewport, which may be letterboxed inside the swap chain.
    void Draw(float swapChainWidth, float swapChainHeight, float viewportOffsetX, float viewportOffsetY);

    // Whether the touch gamepad is on screen and should be reported as a connected controller.
    bool IsActive();

    // Merges the touch gamepad state into a controller state.
    void Apply(XAMINPUT_GAMEPAD& pad);

    // Hides the overlay once a physical controller is used. Touching the screen shows it again.
    void OnPhysicalControllerInput();
}
