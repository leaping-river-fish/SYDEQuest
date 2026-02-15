#pragma once

enum class Button {
    Left,
    Right,
    Jump, 
    Down, 
    Pause
};

class IInput {
public:
    virtual ~IInput() = 0;

    virtual void Update() = 0;
    virtual bool isPressed(Button button) const = 0;
    virtual bool wasJustPressed(Button button) const = 0;
    virtual bool wasJustReleased(Button button) const = 0;
};