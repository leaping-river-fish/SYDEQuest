#pragma once
#include "player/Player.h"
#include "player/PlayerController.h"
#include "level/Level.h"
#include "level/Camera.h"
#include "physics/Physics.h"
#include "physics/Collision.h"

class IRenderer;
class IInput;
class IHaptics;
class ITimer;

class Game {
public:
    Game(IRenderer* renderer, IInput* input, IHaptics* haptics, ITimer* timer);
    
    void init();
    void update();
    void render();
    
private:
    // Platform interfaces
    IRenderer* renderer;
    IInput* input;
    IHaptics* haptics;
    ITimer* timer;
    
    // Game systems
    Player player;
    Level level;
    Camera camera;
    Physics physics;
    Collision collision;
    PlayerController controller;
};

