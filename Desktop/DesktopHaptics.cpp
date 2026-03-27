#include "DesktopHaptics.h"
#include <cstdio>

void DesktopHaptics::trigger(HapticEffect effect, int durationMs) {
    const char* name = "Unknown";
    switch (effect) {
        case HapticEffect::Light: name = "Light"; break;
        case HapticEffect::Medium: name = "Medium"; break;
        case HapticEffect::Heavy: name = "Heavy"; break;
    }
    printf("[HAPTIC] %s for %dms\n", name, durationMs);
}

void DesktopHaptics::stop() {
    // Nothing to do for simulation
}