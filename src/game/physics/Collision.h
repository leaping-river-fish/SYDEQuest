#pragma once

class Player;
class Level;

class Collision {
public:
    void resolveVertical(Player& player, const Level& level);
    void resolveHorizontal(Player& player, const Level& level);
};

