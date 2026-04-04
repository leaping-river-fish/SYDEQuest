#include "DesktopInput.h"

DesktopInput::DesktopInput(SDL_Renderer* sdlRenderer)
    : sdlRenderer(sdlRenderer),
      quit(false),
      mouseLogicalX(0),
      mouseLogicalY(0),
      mousePrimaryDown(false),
      mousePrimaryWasDown(false) {
    for (int i = 0; i < 8; i++) {
        currentState[i] = false;
        previousState[i] = false;
    }
}

void DesktopInput::update() {
    for (int i = 0; i < 8; i++) {
        previousState[i] = currentState[i];
    }
    mousePrimaryWasDown = mousePrimaryDown;

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            quit = true;
        }
    }

    const Uint8* keys = SDL_GetKeyboardState(nullptr);

    currentState[getButtonIndex(Button::Left)] =
        keys[SDL_SCANCODE_A] || keys[SDL_SCANCODE_LEFT];
    currentState[getButtonIndex(Button::Right)] =
        keys[SDL_SCANCODE_D] || keys[SDL_SCANCODE_RIGHT];
    currentState[getButtonIndex(Button::Jump)] =
        keys[SDL_SCANCODE_SPACE] || keys[SDL_SCANCODE_W] || keys[SDL_SCANCODE_UP];
    currentState[getButtonIndex(Button::Down)] =
        keys[SDL_SCANCODE_S] || keys[SDL_SCANCODE_DOWN];
    // Escape: back to title (GP6); same as MenuBack
    currentState[getButtonIndex(Button::Pause)] = keys[SDL_SCANCODE_ESCAPE];
    currentState[getButtonIndex(Button::Fire)] =
        keys[SDL_SCANCODE_X] || keys[SDL_SCANCODE_LCTRL];

    currentState[getButtonIndex(Button::MenuBack)] = keys[SDL_SCANCODE_ESCAPE];
    // Return / Enter: confirm on menu (GP7)
    currentState[getButtonIndex(Button::MenuConfirm)] =
        keys[SDL_SCANCODE_RETURN] || keys[SDL_SCANCODE_KP_ENTER];

    int windowX = 0;
    int windowY = 0;
    Uint32 mouseState = SDL_GetMouseState(&windowX, &windowY);
    float lx = 0.f;
    float ly = 0.f;
    SDL_RenderWindowToLogical(sdlRenderer, static_cast<float>(windowX), static_cast<float>(windowY), &lx, &ly);
    mouseLogicalX = static_cast<int>(lx);
    mouseLogicalY = static_cast<int>(ly);

    mousePrimaryDown = (mouseState & SDL_BUTTON(SDL_BUTTON_LEFT)) != 0;
    currentState[getButtonIndex(Button::Fire)] =
        mousePrimaryDown || keys[SDL_SCANCODE_X] || keys[SDL_SCANCODE_LCTRL];
}

bool DesktopInput::isPressed(Button button) const {
    return currentState[getButtonIndex(button)];
}

bool DesktopInput::wasJustPressed(Button button) const {
    int idx = getButtonIndex(button);
    return currentState[idx] && !previousState[idx];
}

bool DesktopInput::wasJustReleased(Button button) const {
    int idx = getButtonIndex(button);
    return !currentState[idx] && previousState[idx];
}

void DesktopInput::getMouseLogicalPosition(int& outX, int& outY) const {
    outX = mouseLogicalX;
    outY = mouseLogicalY;
}

bool DesktopInput::wasMousePrimaryJustPressed() const {
    return mousePrimaryDown && !mousePrimaryWasDown;
}

int DesktopInput::getButtonIndex(Button button) const {
    return static_cast<int>(button);
}
