#include "PicoTimer.h"
#include "pico/stdlib.h"

PicoTimer::PicoTimer() : lastTime(0), currentTicks(0), deltaTime(0.016f) {
}

void PicoTimer::update() {
    uint64_t currentTime = time_us_64();
    currentTicks = currentTime / 1000;
    
    if (lastTime == 0) {
        lastTime = currentTime;
        deltaTime = 0.016f;
        return;
    }
    
    uint64_t elapsed = currentTime - lastTime;
    deltaTime = elapsed / 1000000.0f;
    
    if (deltaTime > MAX_DELTA_TIME) {
        deltaTime = MAX_DELTA_TIME;
    }
    
    lastTime = currentTime;
}

float PicoTimer::getDeltaTime() const {
    return deltaTime;
}

uint64_t PicoTimer::getTicks() const {
    return currentTicks;
}
