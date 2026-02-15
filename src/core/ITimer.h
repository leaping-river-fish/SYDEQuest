#pragma once
#include <cstdint>

class ITimer {
public:
    virtual ~ITimer() = default;
    
    virtual void update() = 0;
    virtual float getDeltaTime() const = 0;
    virtual uint64_t getTicks() const = 0;
};