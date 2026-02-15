#pragma once

class Player;
class IInput;

class PlayerController {
public:
    void update(Player& player, IInput* input);
    
private:
    void handleMovement(Player& player, IInput* input);
    void handleJump(Player& player, IInput* input);
};

