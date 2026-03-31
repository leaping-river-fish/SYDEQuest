#include "PicoHaptics.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "pico/stdlib.h"

namespace {

// DRV2605 register map (TI datasheet / common drivers)
constexpr uint8_t REG_MODE = 0x01;
constexpr uint8_t REG_LIBRARY = 0x03;
constexpr uint8_t REG_WAVESEQ1 = 0x04;
constexpr uint8_t REG_WAVESEQ2 = 0x05;
constexpr uint8_t REG_WAVESEQ3 = 0x06;
constexpr uint8_t REG_WAVESEQ4 = 0x07;
constexpr uint8_t REG_WAVESEQ5 = 0x08;
constexpr uint8_t REG_WAVESEQ6 = 0x09;
constexpr uint8_t REG_WAVESEQ7 = 0x0A;
constexpr uint8_t REG_WAVESEQ8 = 0x0B;
constexpr uint8_t REG_GO = 0x0C;

/** Internal trigger mode (open-loop library waveforms). */
constexpr uint8_t MODE_INTERNAL_TRIGGER = 0x00;
/** LRA actuator library; use ERM library (e.g. 1–5) if motor type differs. */
constexpr uint8_t LIBRARY_LRA = 0x06;

constexpr uint8_t WAVEFORM_LIGHT = 1;
constexpr uint8_t WAVEFORM_MEDIUM = 47;
constexpr uint8_t WAVEFORM_HEAVY = 51;

} // namespace

bool PicoHaptics::writeRegister(uint8_t reg, uint8_t value) {
    uint8_t buf[2] = {reg, value};
    const int n = i2c_write_blocking(i2c0, DRV2605_ADDR, buf, 2, false);
    return n == 2;
}

PicoHaptics::PicoHaptics() {
    i2c_init(i2c0, I2C_BAUD_HZ);
    gpio_set_function(PIN_I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(PIN_I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(PIN_I2C_SDA);
    gpio_pull_up(PIN_I2C_SCL);

    writeRegister(REG_MODE, MODE_INTERNAL_TRIGGER);
    writeRegister(REG_LIBRARY, LIBRARY_LRA);

    // Clear waveform sequence
    writeRegister(REG_WAVESEQ1, 0);
    writeRegister(REG_WAVESEQ2, 0);
    writeRegister(REG_WAVESEQ3, 0);
    writeRegister(REG_WAVESEQ4, 0);
    writeRegister(REG_WAVESEQ5, 0);
    writeRegister(REG_WAVESEQ6, 0);
    writeRegister(REG_WAVESEQ7, 0);
    writeRegister(REG_WAVESEQ8, 0);
    writeRegister(REG_GO, 0);
}

void PicoHaptics::trigger(HapticEffect effect, int durationMs) {
    (void)durationMs;

    uint8_t waveform;
    switch (effect) {
        case HapticEffect::Light:
            waveform = WAVEFORM_LIGHT;
            break;
        case HapticEffect::Medium:
            waveform = WAVEFORM_MEDIUM;
            break;
        case HapticEffect::Heavy:
            waveform = WAVEFORM_HEAVY;
            break;
        default:
            waveform = WAVEFORM_LIGHT;
            break;
    }

    writeRegister(REG_WAVESEQ1, waveform);
    writeRegister(REG_WAVESEQ2, 0); // end of sequence
    writeRegister(REG_GO, 1);
}

void PicoHaptics::stop() {
    writeRegister(REG_GO, 0);
}
