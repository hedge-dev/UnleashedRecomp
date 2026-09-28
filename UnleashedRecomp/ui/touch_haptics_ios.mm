#include "touch_haptics.h"

#import <UIKit/UIKit.h>

static UIImpactFeedbackGenerator* g_lightFeedback;
static UIImpactFeedbackGenerator* g_mediumFeedback;

namespace TouchHaptics
{
    void Init()
    {
        dispatch_async(dispatch_get_main_queue(), ^{
            g_lightFeedback = [[UIImpactFeedbackGenerator alloc] initWithStyle:UIImpactFeedbackStyleLight];
            g_mediumFeedback = [[UIImpactFeedbackGenerator alloc] initWithStyle:UIImpactFeedbackStyleMedium];
            [g_lightFeedback prepare];
            [g_mediumFeedback prepare];
        });
    }

    void Play(bool light)
    {
        // UIKit feedback generators must only be used from the main thread.
        dispatch_async(dispatch_get_main_queue(), ^{
            UIImpactFeedbackGenerator* generator = light ? g_lightFeedback : g_mediumFeedback;
            [generator impactOccurred];
            [generator prepare];
        });
    }
}
