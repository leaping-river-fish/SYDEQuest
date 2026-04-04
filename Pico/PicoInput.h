#pragma once
#include "../Shared/core/IInput.h"
#include <cstdint>

class PicoInput : public IInput {
public:
    /** GP14 — fire button (active low, internal pull-up). */
    static constexpr int kFireGpioPin = 14;

    PicoInput();

    void update() override;
    bool isPressed(Button button) const override;
    bool wasJustPressed(Button button) const override;
    bool wasJustReleased(Button button) const override;

    void getMouseLogicalPosition(int& outX, int& outY) const override;
    bool wasMousePrimaryJustPressed() const override;

private:
    uint16_t currentState;
    uint16_t previousState;

    static constexpr int PIN_JOY_X = 26;
    static constexpr int PIN_JOY_Y = 27;
    static constexpr int PIN_FIRE = kFireGpioPin;
    /** GP6 — back to title (active low). */
    static constexpr int PIN_MENU_BACK = 6;
    /** GP7 — menu confirm / start / retry (active low). */
    static constexpr int PIN_MENU_CONFIRM = 7;

    static constexpr uint16_t LOW_THRESHOLD = 800;
    static constexpr uint16_t HIGH_THRESHOLD = 3200;

    int buttonToBit(Button button) const;
};
