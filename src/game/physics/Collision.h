#pragma once

class Player;
class Projectile;
class EnemyProjectile;
class Level;

class Collision {
public:
    void resolveVertical(Player& player, const Level& level);
    void resolveHorizontal(Player& player, const Level& level);
    bool checkProjectileTileCollision(const Projectile& projectile, const Level& level);
    bool checkEnemyProjectileTileCollision(const EnemyProjectile& projectile, const Level& level);
};

