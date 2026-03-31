#pragma once

#include "../../core/types.h"

class Player;
class Projectile;
class EnemyProjectile;
class Level;

class Collision {
public:
    void resolveVertical(Player& player, const Level& level, fixed_t prevY);
    void resolveHorizontal(Player& player, const Level& level);
    bool checkProjectileTileCollision(const Projectile& projectile, const Level& level);
    bool checkEnemyProjectileTileCollision(const EnemyProjectile& projectile, const Level& level);
};

