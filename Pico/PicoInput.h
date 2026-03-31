#pragma once
#include "../Shared/core/IInput.h"
#include <cstdint>

class PicoInput : public IInput {
public:
    PicoInput();

    void update() override;
    bool isPressed(Button button) const override;
    bool wasJustPressed(Button button) const override;
    bool wasJustReleased(Button button) const override;

private:
    uint16_t currentState;
    uint16_t previousState;

    static constexpr int PIN_JOY_X = 26;
    static constexpr int PIN_JOY_Y = 27;
    static constexpr int PIN_FIRE = 6;
    static constexpr int PIN_PAUSE = 7;

    /** Wider neutral band (800–3200) reduces spurious Jump/Down from ADC noise at rest. */
    static constexpr uint16_t LOW_THRESHOLD = 800;
    static constexpr uint16_t HIGH_THRESHOLD = 3200;

    int buttonToBit(Button button) const;
};
