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
    
    static constexpr int PIN_UP = 0;
    static constexpr int PIN_DOWN = 1;
    static constexpr int PIN_LEFT = 2;
    static constexpr int PIN_RIGHT = 3;
    static constexpr int PIN_A = 4;
    static constexpr int PIN_B = 5;
    static constexpr int PIN_START = 6;
    static constexpr int PIN_SELECT = 7;
    
    int buttonToBit(Button button) const;
};
