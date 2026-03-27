#include "PicoHaptics.h"
#include "pico/stdlib.h"
#include "hardware/pwm.h"

PicoHaptics::PicoHaptics() {
    gpio_set_function(PIN_HAPTIC, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(PIN_HAPTIC);
    pwm_set_wrap(slice_num, 65535);
    pwm_set_enabled(slice_num, true);
}

void PicoHaptics::trigger(HapticEffect effect, int durationMs) {
    uint slice_num = pwm_gpio_to_slice_num(PIN_HAPTIC);
    
    uint16_t level;
    switch (effect) {
        case HapticEffect::Light:  level = 16383; break;
        case HapticEffect::Medium: level = 32767; break;
        case HapticEffect::Heavy:  level = 65535; break;
        default: level = 0; break;
    }
    
    pwm_set_gpio_level(PIN_HAPTIC, level);
}

void PicoHaptics::stop() {
    pwm_set_gpio_level(PIN_HAPTIC, 0);
}
