#include "PicoInput.h"
#include "hardware/adc.h"
#include "hardware/gpio.h"
#include "pico/stdlib.h"

PicoInput::PicoInput() : currentState(0), previousState(0) {
    adc_init();
    adc_gpio_init(PIN_JOY_X);
    adc_gpio_init(PIN_JOY_Y);

    gpio_init(PIN_FIRE);
    gpio_init(PIN_MENU_BACK);
    gpio_init(PIN_MENU_CONFIRM);
    gpio_set_dir(PIN_FIRE, GPIO_IN);
    gpio_set_dir(PIN_MENU_BACK, GPIO_IN);
    gpio_set_dir(PIN_MENU_CONFIRM, GPIO_IN);
    gpio_pull_up(PIN_FIRE);
    gpio_pull_up(PIN_MENU_BACK);
    gpio_pull_up(PIN_MENU_CONFIRM);
}

int PicoInput::buttonToBit(Button button) const {
    switch (button) {
        case Button::Left: return 0;
        case Button::Right: return 1;
        case Button::Jump: return 2;
        case Button::Down: return 3;
        case Button::Pause: return 6;  // same physical as MenuBack on Pico (GP6)
        case Button::Fire: return 5;
        case Button::MenuBack: return 6;
        case Button::MenuConfirm: return 7;
        default: return -1;
    }
}

void PicoInput::update() {
    gpio_init(PIN_FIRE);
    gpio_set_dir(PIN_FIRE, GPIO_IN);
    gpio_pull_up(PIN_FIRE);

    previousState = currentState;
    currentState = 0;

    adc_select_input(0);
    const uint16_t joyX = adc_read();
    adc_select_input(1);
    const uint16_t joyYRaw = adc_read();
    const uint16_t joyY = 4095 - joyYRaw;

    if (joyX < LOW_THRESHOLD) {
        currentState |= (1 << 0);
    }
    if (joyX > HIGH_THRESHOLD) {
        currentState |= (1 << 1);
    }
    if (joyY < LOW_THRESHOLD) {
        currentState |= (1 << 3);
    }
    if (joyY > HIGH_THRESHOLD) {
        currentState |= (1 << 2);
    }

    if (!gpio_get(PIN_FIRE)) {
        currentState |= (1 << 5);
    }
    if (!gpio_get(PIN_MENU_BACK)) {
        currentState |= (1 << 6);
    }
    if (!gpio_get(PIN_MENU_CONFIRM)) {
        currentState |= (1 << 7);
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

void PicoInput::getMouseLogicalPosition(int& outX, int& outY) const {
    outX = 0;
    outY = 0;
}

bool PicoInput::wasMousePrimaryJustPressed() const {
    return false;
}
