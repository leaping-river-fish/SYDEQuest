/*
 * Boss AI extension points: implement behavior in updateSummoner(), updateLaser(), and
 * updateBulletHell() for BossType::SUMMONER, LASER, and BULLET_HELL respectively.
 */
#include "BossEnemy.h"
#include "../level/Level.h"
#include "../player/Player.h"

BossEnemy::BossEnemy()
    : position(0.0f, 0.0f)
    , health(20)
    , active(true)
    , activated(false)
    , type(BossType::SUMMONER) {}

BossEnemy::BossEnemy(Vec2 spawn, BossType bossType)
    : position(spawn)
    , health(20)
    , active(true)
    , activated(false)
    , type(bossType) {}

void BossEnemy::update(float deltaTime, const Level& level, const Player& player) {
    switch (type) {
        case BossType::SUMMONER:
            updateSummoner(deltaTime, level, player);
            break;
        case BossType::LASER:
            updateLaser(deltaTime, level, player);
            break;
        case BossType::BULLET_HELL:
            updateBulletHell(deltaTime, level, player);
            break;
    }
}

void BossEnemy::updateSummoner(float /*deltaTime*/, const Level& /*level*/, const Player& /*player*/) {}

void BossEnemy::updateLaser(float /*deltaTime*/, const Level& /*level*/, const Player& /*player*/) {}

void BossEnemy::updateBulletHell(float /*deltaTime*/, const Level& /*level*/, const Player& /*player*/) {}

bool BossEnemy::takeDamage(int amount) {
    health -= amount;
    if (health <= 0) {
        health = 0;
        active = false;
        return true;
    }
    return false;
}

Rect BossEnemy::getCollider() const {
    return Rect(position.x, position.y, WIDTH, HEIGHT);
}

void BossEnemy::computeArenaWalls(Rect out[4]) const {
#ifdef PLATFORM_PICO
    fixed_t cx = position.x + FIXED_DIV(WIDTH, TO_FIXED(2.0f));
    fixed_t cy = position.y + FIXED_DIV(HEIGHT, TO_FIXED(2.0f));
    fixed_t t = ARENA_WALL_THICKNESS;
    fixed_t w2 = FIXED_MUL(ARENA_HALF_W, TO_FIXED(2.0f));
    fixed_t h2 = FIXED_MUL(ARENA_HALF_H, TO_FIXED(2.0f));

    out[0] = Rect(cx - ARENA_HALF_W - t, cy - ARENA_HALF_H, t, h2);
    out[1] = Rect(cx + ARENA_HALF_W, cy - ARENA_HALF_H, t, h2);
    out[2] = Rect(cx - ARENA_HALF_W, cy - ARENA_HALF_H - t, w2, t);
    out[3] = Rect(cx - ARENA_HALF_W, cy + ARENA_HALF_H, w2, t);
#else
    float cx = position.x + WIDTH * 0.5f;
    float cy = position.y + HEIGHT * 0.5f;
    float t = ARENA_WALL_THICKNESS;
    float w2 = ARENA_HALF_W * 2.0f;
    float h2 = ARENA_HALF_H * 2.0f;

    out[0] = Rect(cx - ARENA_HALF_W - t, cy - ARENA_HALF_H, t, h2);
    out[1] = Rect(cx + ARENA_HALF_W, cy - ARENA_HALF_H, t, h2);
    out[2] = Rect(cx - ARENA_HALF_W, cy - ARENA_HALF_H - t, w2, t);
    out[3] = Rect(cx - ARENA_HALF_W, cy + ARENA_HALF_H, w2, t);
#endif
}
