/*
 * Boss AI extension points: implement behavior in updateSummoner(), updateLaser(), and
 * updateBulletHell() for BossType::SUMMONER, LASER, and BULLET_HELL respectively.
 */
#include "BossEnemy.h"
#include "../Game.h"
#include "../level/Level.h"
#include "../player/Player.h"
#include <cmath>

#ifdef PLATFORM_PICO
static constexpr int kBossDefaultHp = 20;
static constexpr int kBossSummonerHpPico = 30;
/** Pico: lower than desktop (50) for shorter laser arena fights. */
static constexpr int kBossLaserHpPico = 30;
#else
static constexpr int kBossDefaultHp = 50;
#endif

static int bossInitialHp(BossType t) {
#ifdef PLATFORM_PICO
    if (t == BossType::SUMMONER) {
        return kBossSummonerHpPico;
    }
    if (t == BossType::LASER) {
        return kBossLaserHpPico;
    }
    return kBossDefaultHp;
#else
    (void)t;
    return kBossDefaultHp;
#endif
}

BossEnemy::BossEnemy()
    : position(0.0f, 0.0f)
    , health(bossInitialHp(BossType::SUMMONER))
    , maxHealth(bossInitialHp(BossType::SUMMONER))
    , active(true)
    , activated(false)
    , type(BossType::SUMMONER)
    , summonerState(BossState::Moving)
    , currentWaypoint(0)
    , stateTimer(0.0f)
    , summonerSpawnedThisPhase(false)
    , laserWaypointOrder{0, 1, 2, 3, 4}
    , laserWaypointIndex(0)
    , laserOrderInitialized(false)
    , laserRngState(0xC0FFEEu)
    , laserFireTimer(0.0f)
    , laserWaypointPauseRemaining(0.0f)
    , laserArenaCenterX(0)
    , laserArenaCenterY(0) {}

BossEnemy::BossEnemy(Vec2 spawn, BossType bossType)
    : position(spawn)
    , health(bossInitialHp(bossType))
    , maxHealth(bossInitialHp(bossType))
    , active(true)
    , activated(false)
    , type(bossType)
    , summonerState(BossState::Moving)
    , currentWaypoint(0)
    , stateTimer(0.0f)
    , summonerSpawnedThisPhase(false)
    , laserWaypointOrder{0, 1, 2, 3, 4}
    , laserWaypointIndex(0)
    , laserOrderInitialized(false)
    , laserRngState(0xC0FFEEu)
    , laserFireTimer(0.0f)
    , laserWaypointPauseRemaining(0.0f)
    , laserArenaCenterX(0)
    , laserArenaCenterY(0) {
    if (bossType == BossType::LASER) {
        laserFireTimer = BossEnemy::LASER_FIRE_INTERVAL_SEC;
        laserArenaCenterX = spawn.x + FIXED_DIV(WIDTH, TO_FIXED(2.0f));
        laserArenaCenterY = spawn.y + FIXED_DIV(HEIGHT, TO_FIXED(2.0f));
    }
}

void BossEnemy::update(float deltaTime, const Level& level, const Player& player) {
    switch (type) {
        case BossType::BULLET_HELL:
            updateBulletHell(deltaTime, level, player);
            break;
        case BossType::SUMMONER:
        case BossType::LASER:
        default:
            break;
    }
}

void BossEnemy::updateLaser(float dt, const Level&, const Player& player, Game& game) {
    if (type != BossType::LASER || !activated) {
        return;
    }
    if (game.hasActiveLaser()) {
        return;
    }
    laserFireTimer -= dt;
    if (laserFireTimer > 0.0f) {
        return;
    }
    laserFireTimer = LASER_FIRE_INTERVAL_SEC;

    Vec2 origin;
    origin.x = position.x + FIXED_DIV(WIDTH, TO_FIXED(2.0f));
    origin.y = position.y + FIXED_DIV(HEIGHT, TO_FIXED(2.0f));
    float px = fixedToFloat(player.position.x) + fixedToFloat(Player::WIDTH) * 0.5f;
    float py = fixedToFloat(player.position.y) + fixedToFloat(Player::HEIGHT) * 0.5f;
    float ox = fixedToFloat(origin.x);
    float oy = fixedToFloat(origin.y);
    float dx = px - ox;
    float dy = py - oy;
    float len = std::sqrt(dx * dx + dy * dy);
    if (len < 1e-4f) {
        return;
    }
    game.spawnLaser(*this, origin, dx / len, dy / len);
}

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
    fixed_t cx;
    fixed_t cy;
    if (type == BossType::LASER) {
        cx = laserArenaCenterX;
        cy = laserArenaCenterY;
    } else {
        cx = position.x + FIXED_DIV(WIDTH, TO_FIXED(2.0f));
        cy = position.y + FIXED_DIV(HEIGHT, TO_FIXED(2.0f));
    }
    fixed_t t = ARENA_WALL_THICKNESS;
    fixed_t halfW = ARENA_HALF_W;
    fixed_t halfH = ARENA_HALF_H;
    if (type == BossType::LASER) {
        halfW = TO_FIXED(LASER_ARENA_HALF_W_PX);
        halfH = TO_FIXED(LASER_ARENA_HALF_H_PX);
    }
    fixed_t w2 = FIXED_MUL(halfW, TO_FIXED(2.0f));
    fixed_t h2 = FIXED_MUL(halfH, TO_FIXED(2.0f));

    out[0] = Rect(cx - halfW - t, cy - halfH, t, h2);
    out[1] = Rect(cx + halfW, cy - halfH, t, h2);
    out[2] = Rect(cx - halfW, cy - halfH - t, w2, t);
    out[3] = Rect(cx - halfW, cy + halfH, w2, t);
#else
    float cx;
    float cy;
    if (type == BossType::LASER) {
        cx = laserArenaCenterX;
        cy = laserArenaCenterY;
    } else {
        cx = position.x + WIDTH * 0.5f;
        cy = position.y + HEIGHT * 0.5f;
    }
    float t = ARENA_WALL_THICKNESS;
    float halfW = ARENA_HALF_W;
    float halfH = ARENA_HALF_H;
    if (type == BossType::LASER) {
        halfW = LASER_ARENA_HALF_W_PX;
        halfH = LASER_ARENA_HALF_H_PX;
    }
    float w2 = halfW * 2.0f;
    float h2 = halfH * 2.0f;

    out[0] = Rect(cx - halfW - t, cy - halfH, t, h2);
    out[1] = Rect(cx + halfW, cy - halfH, t, h2);
    out[2] = Rect(cx - halfW, cy - halfH - t, w2, t);
    out[3] = Rect(cx - halfW, cy + halfH, w2, t);
#endif
}
