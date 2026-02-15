#pragma once
#include "core/IHaptics.h"

class DesktopHaptics : public IHaptics {
public:
    void trigger(HapticEffect effect, int durationMs) override;
    void stop() override;
};

