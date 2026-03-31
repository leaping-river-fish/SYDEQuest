#pragma once
#include "../Shared/core/IHaptics.h"
#include <cstdint>

class PicoHaptics : public IHaptics {
public:
    PicoHaptics();

    void trigger(HapticEffect effect, int durationMs) override;
    void stop() override;

private:
    static constexpr int PIN_I2C_SDA = 0;
    static constexpr int PIN_I2C_SCL = 1;
    static constexpr uint32_t I2C_BAUD_HZ = 400000;
    static constexpr uint8_t DRV2605_ADDR = 0x5A;

    bool writeRegister(uint8_t reg, uint8_t value);
};
