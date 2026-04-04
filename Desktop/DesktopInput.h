#pragma once
#include "../Shared/core/IInput.h"
#include <SDL2/SDL.h>

class DesktopInput : public IInput {
public:
    explicit DesktopInput(SDL_Renderer* sdlRenderer);

    void update() override;
    bool isPressed(Button button) const override;
    bool wasJustPressed(Button button) const override;
    bool wasJustReleased(Button button) const override;

    void getMouseLogicalPosition(int& outX, int& outY) const override;
    bool wasMousePrimaryJustPressed() const override;

    bool shouldQuit() const { return quit; }

private:
    SDL_Renderer* sdlRenderer;
    bool currentState[8];
    bool previousState[8];
    bool quit;

    int mouseLogicalX;
    int mouseLogicalY;
    bool mousePrimaryDown;
    bool mousePrimaryWasDown;

    int getButtonIndex(Button button) const;
};
