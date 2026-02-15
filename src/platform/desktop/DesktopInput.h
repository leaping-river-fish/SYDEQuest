#pragma once
#include "core/IInput.h"
#include <SDL2/SDL.h>

class DesktopInput : public IInput {
public:
    DesktopInput();
    
    void update() override;
    bool isPressed(Button button) const override;
    bool wasJustPressed(Button button) const override;
    bool wasJustReleased(Button button) const override;
    
    bool shouldQuit() const { return quit; }
    
private:
    bool currentState[5];
    bool previousState[5];
    bool quit;
    
    int getButtonIndex(Button button) const;
};

