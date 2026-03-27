#include "PicoInput.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"

PicoInput::PicoInput() : currentState(0), previousState(0) {
    gpio_init(PIN_LEFT);
    gpio_init(PIN_RIGHT);
    gpio_init(PIN_A);
    gpio_init(PIN_DOWN);
    gpio_init(PIN_START);
    gpio_init(PIN_B);
    
    gpio_set_dir(PIN_LEFT, GPIO_IN);
    gpio_set_dir(PIN_RIGHT, GPIO_IN);
    gpio_set_dir(PIN_A, GPIO_IN);
    gpio_set_dir(PIN_DOWN, GPIO_IN);
    gpio_set_dir(PIN_START, GPIO_IN);
    gpio_set_dir(PIN_B, GPIO_IN);
    
    gpio_pull_up(PIN_LEFT);
    gpio_pull_up(PIN_RIGHT);
    gpio_pull_up(PIN_A);
    gpio_pull_up(PIN_DOWN);
    gpio_pull_up(PIN_START);
    gpio_pull_up(PIN_B);
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
    previousState = currentState;
    currentState = 0;
    
    if (!gpio_get(PIN_LEFT)) currentState |= (1 << 0);
    if (!gpio_get(PIN_RIGHT)) currentState |= (1 << 1);
    if (!gpio_get(PIN_A)) currentState |= (1 << 2);
    if (!gpio_get(PIN_DOWN)) currentState |= (1 << 3);
    if (!gpio_get(PIN_START)) currentState |= (1 << 4);
    if (!gpio_get(PIN_B)) currentState |= (1 << 5);
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
