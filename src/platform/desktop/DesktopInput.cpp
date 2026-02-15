#include "DesktopInput.h"

DesktopInput::DesktopInput() : quit(false) {
    for (int i = 0; i < 5; i++) {
        currentState[i] = false;
        previousState[i] = false;
    }
}

void DesktopInput::update() {
    // Save previous state
    for (int i = 0; i < 5; i++) {
        previousState[i] = currentState[i];
    }
    
    // Poll SDL events
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            quit = true;
        }
    }
    
    // Read keyboard state
    const Uint8* keys = SDL_GetKeyboardState(nullptr);
    
    currentState[getButtonIndex(Button::Left)] = 
        keys[SDL_SCANCODE_A] || keys[SDL_SCANCODE_LEFT];
    currentState[getButtonIndex(Button::Right)] = 
        keys[SDL_SCANCODE_D] || keys[SDL_SCANCODE_RIGHT];
    currentState[getButtonIndex(Button::Jump)] = 
        keys[SDL_SCANCODE_SPACE] || keys[SDL_SCANCODE_W] || keys[SDL_SCANCODE_UP];
    currentState[getButtonIndex(Button::Down)] = 
        keys[SDL_SCANCODE_S] || keys[SDL_SCANCODE_DOWN];
    currentState[getButtonIndex(Button::Pause)] = 
        keys[SDL_SCANCODE_ESCAPE];
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

int DesktopInput::getButtonIndex(Button button) const {
    return static_cast<int>(button);
}

