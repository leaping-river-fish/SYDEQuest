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
    virtual ~IInput() = default;

    virtual void update() = 0;
    virtual bool isPressed(Button button) const = 0;
    virtual bool wasJustPressed(Button button) const = 0;
    virtual bool wasJustReleased(Button button) const = 0;
};