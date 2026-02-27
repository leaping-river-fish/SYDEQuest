#pragma once
#include "core/ITimer.h"
#include <SDL2/SDL.h>

class DesktopTimer : public ITimer {
public:
    DesktopTimer();
    
    void update() override;
    float getDeltaTime() const override { return deltaTime; }
    uint64_t getTicks() const override { return lastTicks; }
    
private:
    uint64_t lastTicks;
    float deltaTime;
};