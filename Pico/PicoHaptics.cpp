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
constexpr uint8_t REG_FEEDBACK = 0x1A;
constexpr uint8_t REG_CONTROL3 = 0x1F;

/** Internal trigger mode (open-loop library waveforms). */
constexpr uint8_t MODE_INTERNAL_TRIGGER = 0x00;
/** TI closed-loop ERM library 1. */
constexpr uint8_t LIBRARY_ERM = 0x01;
/** ERM max drive: bits 6–0 = 0x7F, bit 7 = 0 (ERM vs LRA). */
constexpr uint8_t FEEDBACK_ERM_MAX = 0x7F;
/** Control3: clear STANDBY (bit 6). */
constexpr uint8_t CONTROL3_ACTIVE = 0x00;

/** TI Library 1 — safe low indices for coin ERM. */
constexpr uint8_t WAVEFORM_LIGHT = 1;
constexpr uint8_t WAVEFORM_MEDIUM = 2;
constexpr uint8_t WAVEFORM_HEAVY = 3;

constexpr int kPowerStabilizeMs = 250;
constexpr int kStartupHapticMs = 100;

/** Per-transaction cap so a missing/faulty DRV2605 cannot hang the game thread indefinitely. */
constexpr uint32_t kI2cWriteTimeoutUs = 5000;

} // namespace

bool PicoHaptics::writeRegister(uint8_t reg, uint8_t value) {
    uint8_t buf[2] = {reg, value};
    const int n = i2c_write_timeout_us(i2c0, DRV2605_ADDR, buf, 2, false, kI2cWriteTimeoutUs);
    return n == 2;
}

PicoHaptics::PicoHaptics() {
    i2c_init(i2c0, I2C_BAUD_HZ);
    gpio_set_function(PIN_I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(PIN_I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(PIN_I2C_SDA);
    gpio_pull_up(PIN_I2C_SCL);

    sleep_ms(15);

    writeRegister(REG_CONTROL3, CONTROL3_ACTIVE);
    writeRegister(REG_FEEDBACK, FEEDBACK_ERM_MAX);
    writeRegister(REG_LIBRARY, LIBRARY_ERM);
    writeRegister(REG_MODE, MODE_INTERNAL_TRIGGER);

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

    sleep_ms(kPowerStabilizeMs);
    trigger(HapticEffect::Heavy, kStartupHapticMs);
    sleep_ms(kStartupHapticMs);
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

    // Abort sequence if the bus is stuck — each write is capped by kI2cWriteTimeoutUs.
    if (!writeRegister(REG_GO, 0)) {
        return;
    }
    if (!writeRegister(REG_MODE, MODE_INTERNAL_TRIGGER)) {
        return;
    }
    if (!writeRegister(REG_WAVESEQ1, waveform)) {
        return;
    }
    if (!writeRegister(REG_WAVESEQ2, 0)) { // end of sequence
        return;
    }
    // No sleep_ms here: blocking delays on the gameplay path cause frame hitches (see gameplay freeze audit).
    (void)writeRegister(REG_GO, 1);
}

void PicoHaptics::stop() {
    writeRegister(REG_GO, 0);
}
