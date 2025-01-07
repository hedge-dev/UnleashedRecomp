#include "exports_window.h"
#include <ui/reddog/reddog_controls.h>
#include <user/config.h>

static ExportsWindow g_window{ "Exports" };

void ExportsWindow::Draw()
{
    if (Begin())
    {
        ImGui::TextColored({ 1, 0, 0, 1 }, "For testing purposes only, use Hedge Mod Manager to toggle these.");
        Reddog::Checkbox("Allow Cancelling Unleash", &Config::AllowCancellingUnleash.Value);
        Reddog::Checkbox("Fix Unleash Out of Control Drain", &Config::FixUnleashOutOfControlDrain.Value);
        Reddog::Checkbox("Homing Attack on Boost", &Config::HomingAttackOnBoost.Value);
        Reddog::Checkbox("Save Score at Checkpoints", &Config::SaveScoreAtCheckpoints.Value);
        Reddog::Checkbox("Skip Intro Logos", &Config::SkipIntroLogos.Value);
    }
    End();
}
