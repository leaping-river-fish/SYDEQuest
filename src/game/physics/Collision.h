#pragma once

class Player;
class Projectile;
class Level;

class Collision {
public:
    void resolveVertical(Player& player, const Level& level);
    void resolveHorizontal(Player& player, const Level& level);
    bool checkProjectileTileCollision(const Projectile& projectile, const Level& level);
};

