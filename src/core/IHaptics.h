#pragma once

enum class HapticEffect {
    Light,
    Medium,
    Heavy
};

class IHaptics {
public:
    virtual ~IHaptics() = default;
    
    virtual void trigger(HapticEffect effect, int durationMs) = 0;
    virtual void stop() = 0;
};

