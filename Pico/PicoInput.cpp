#include "PicoInput.h"
#include "hardware/adc.h"
#include "hardware/gpio.h"
#include "pico/stdlib.h"

PicoInput::PicoInput() : currentState(0), previousState(0) {
    adc_init();
    adc_gpio_init(PIN_JOY_X);
    adc_gpio_init(PIN_JOY_Y);

    gpio_init(PIN_FIRE);
    gpio_init(PIN_PAUSE);
    gpio_set_dir(PIN_FIRE, GPIO_IN);
    gpio_set_dir(PIN_PAUSE, GPIO_IN);
    gpio_pull_up(PIN_FIRE);
    gpio_pull_up(PIN_PAUSE);
}

int PicoInput::buttonToBit(Button button) const {
    switch (button) {
        case Button::Left: return 0;
        case Button::Right: return 1;
        case Button::Jump: return 2;
        case Button::Down: return 3;
        case Button::Pause: return 4;
        case Button::Fire: return 5;
        default: return -1;
    }
}

void PicoInput::update() {
    // Re-assert GP14 as SIO input + pull-up every frame so nothing else can leave it misconfigured.
    gpio_init(PIN_FIRE);
    gpio_set_dir(PIN_FIRE, GPIO_IN);
    gpio_pull_up(PIN_FIRE);

    previousState = currentState;
    currentState = 0;

    adc_select_input(0);
    const uint16_t joyX = adc_read();
    adc_select_input(1);
    const uint16_t joyYRaw = adc_read();
    // Hardware Y is inverted (physical up = low ADC, down = high). Normalize so joyY rises with physical up.
    const uint16_t joyY = 4095 - joyYRaw;

    if (joyX < LOW_THRESHOLD) {
        currentState |= (1 << 0);
    }
    if (joyX > HIGH_THRESHOLD) {
        currentState |= (1 << 1);
    }
    if (joyY < LOW_THRESHOLD) {
        currentState |= (1 << 3);  // Button::Down
    }
    if (joyY > HIGH_THRESHOLD) {
        currentState |= (1 << 2);  // Button::Jump
    }

    if (!gpio_get(PIN_FIRE)) {
        currentState |= (1 << 5);
    }
    if (!gpio_get(PIN_PAUSE)) {
        currentState |= (1 << 4);
    }
}

bool PicoInput::isPressed(Button button) const {
    int bit = buttonToBit(button);
    if (bit < 0) return false;
    return currentState & (1 << bit);
}

bool PicoInput::wasJustPressed(Button button) const {
    int bit = buttonToBit(button);
    if (bit < 0) return false;
    return (currentState & (1 << bit)) && !(previousState & (1 << bit));
}

bool PicoInput::wasJustReleased(Button button) const {
    int bit = buttonToBit(button);
    if (bit < 0) return false;
    return !(currentState & (1 << bit)) && (previousState & (1 << bit));
}
