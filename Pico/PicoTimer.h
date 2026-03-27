#pragma once
#include "../Shared/core/ITimer.h"
#include <cstdint>

class PicoTimer : public ITimer {
public:
    PicoTimer();
    
    void update() override;
    float getDeltaTime() const override;
    uint64_t getTicks() const override;
    
private:
    uint64_t lastTime;
    uint64_t currentTicks;
    float deltaTime;
    
    static constexpr float MAX_DELTA_TIME = 0.1f;
};
