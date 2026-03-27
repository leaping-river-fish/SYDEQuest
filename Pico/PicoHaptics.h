#pragma once
#include "../Shared/core/IHaptics.h"
#include <cstdint>

class PicoHaptics : public IHaptics {
public:
    PicoHaptics();
    
    void trigger(HapticEffect effect, int durationMs) override;
    void stop() override;
    
private:
    static constexpr int PIN_HAPTIC = 12;
};
