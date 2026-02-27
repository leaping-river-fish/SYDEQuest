#include "DesktopTimer.h"

DesktopTimer::DesktopTimer() 
    : lastTicks(SDL_GetTicks64()), deltaTime(0.016f)
{
}

void DesktopTimer::update() {
    uint64_t now = SDL_GetTicks64();
    deltaTime = (now - lastTicks) / 1000.0f;
    
    // Clamp to prevent huge jumps
    if (deltaTime > 0.1f) {
        deltaTime = 0.016f;
    }
    
    lastTicks = now;
}