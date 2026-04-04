#pragma once

#include "../../core/types.h"
#include <cstddef>

class Player;
class Projectile;
class EnemyProjectile;
class Level;

class Collision {
public:
    void resolveVertical(Player& player, const Level& level, fixed_t prevY);
    void resolveHorizontal(Player& player, const Level& level);
    /** Push player out of axis-aligned arena segments (after tile resolution). */
    void resolveArenaWalls(Player& player, const Rect* walls, size_t count);
    bool checkProjectileTileCollision(const Projectile& projectile, const Level& level);
    bool checkEnemyProjectileTileCollision(const EnemyProjectile& projectile, const Level& level);
};

