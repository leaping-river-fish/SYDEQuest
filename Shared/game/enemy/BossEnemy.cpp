/*
 * Boss AI extension points: implement behavior in updateSummoner(), updateLaser(), and
 * updateBulletHell() for BossType::SUMMONER, LASER, and BULLET_HELL respectively.
 */
#include "BossEnemy.h"
#include "../Game.h"
#include "../level/Level.h"
#include "../player/Player.h"
#include "../enemy/EnemyProjectile.h"
#include <algorithm>
#include <cmath>

#ifdef PLATFORM_PICO
/** Default for bosses without a dedicated value (e.g. BULLET_HELL); matches arena bosses at 30. */
static constexpr int kBossDefaultHp = 30;
static constexpr int kBossSummonerHpPico = 30;
/** Pico: lower than desktop (50) for shorter laser arena fights. */
static constexpr int kBossLaserHpPico = 30;
#else
static constexpr int kBossDefaultHp = 50;
#endif

namespace {

void clampBossCenterToBulletHellArena(BossEnemy& boss) {
    float acx = fixedToFloat(boss.bulletHellArenaCenterX);
    float acy = fixedToFloat(boss.bulletHellArenaCenterY);
    float hafW = BossEnemy::BULLET_HELL_ARENA_HALF_W_PX;
    float hafH = BossEnemy::BULLET_HELL_ARENA_HALF_H_PX;
    float hw = fixedToFloat(BossEnemy::WIDTH) * 0.5f;
    float hh = fixedToFloat(BossEnemy::HEIGHT) * 0.5f;
    float minCx = acx - hafW + hw;
    float maxCx = acx + hafW - hw;
    float minCy = acy - hafH + hh;
    float maxCy = acy + hafH - hh;
    float cx = fixedToFloat(boss.position.x) + hw;
    float cy = fixedToFloat(boss.position.y) + hh;
    if (cx < minCx) {
        cx = minCx;
    }
    if (cx > maxCx) {
        cx = maxCx;
    }
    if (cy < minCy) {
        cy = minCy;
    }
    if (cy > maxCy) {
        cy = maxCy;
    }
    boss.position.x = floatToFixed(cx - hw);
    boss.position.y = floatToFixed(cy - hh);
}

}  // namespace

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
    , postActivationIdleRemaining(0.0f)
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
    , laserArenaCenterY(0)
    , bulletHellArenaCenterX(0)
    , bulletHellArenaCenterY(0)
    , bulletHellState(BulletHellState::ProjectilePhase)
    , bulletHellStateTimer(0.0f)
    , chargeAttempts(0)
    , bulletHellChargeSub(BulletHellChargeSub::Warning)
    , chargeDirX(0.0f)
    , chargeDirY(1.0f)
    , dashLockPlayerX(0.0f)
    , dashLockPlayerY(0.0f)
    , dashTotalDistance(0.0f)
    , dashDistanceTraveled(0.0f) {}

BossEnemy::BossEnemy(Vec2 spawn, BossType bossType)
    : position(spawn)
    , health(bossInitialHp(bossType))
    , maxHealth(bossInitialHp(bossType))
    , active(true)
    , activated(false)
    , postActivationIdleRemaining(0.0f)
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
    , laserArenaCenterY(0)
    , bulletHellArenaCenterX(0)
    , bulletHellArenaCenterY(0)
    , bulletHellState(BulletHellState::ProjectilePhase)
    , bulletHellStateTimer(0.0f)
    , chargeAttempts(0)
    , bulletHellChargeSub(BulletHellChargeSub::Warning)
    , chargeDirX(0.0f)
    , chargeDirY(1.0f)
    , dashLockPlayerX(0.0f)
    , dashLockPlayerY(0.0f)
    , dashTotalDistance(0.0f)
    , dashDistanceTraveled(0.0f) {
    if (bossType == BossType::LASER) {
        laserFireTimer = BossEnemy::LASER_FIRE_INTERVAL_SEC;
        laserArenaCenterX = spawn.x + FIXED_DIV(WIDTH, TO_FIXED(2.0f));
        laserArenaCenterY = spawn.y + FIXED_DIV(HEIGHT, TO_FIXED(2.0f));
    }
    if (bossType == BossType::BULLET_HELL) {
        bulletHellArenaCenterX = spawn.x + FIXED_DIV(WIDTH, TO_FIXED(2.0f));
        bulletHellArenaCenterY = spawn.y + FIXED_DIV(HEIGHT, TO_FIXED(2.0f));
        bulletHellState = BulletHellState::ProjectilePhase;
        bulletHellStateTimer = 0.0f;
        chargeAttempts = 0;
        bulletHellChargeSub = BulletHellChargeSub::Warning;
    }
}

void BossEnemy::update(float deltaTime, const Level& level, const Player& player) {
    switch (type) {
        case BossType::BULLET_HELL:
            (void)deltaTime;
            (void)level;
            (void)player;
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

void BossEnemy::updateBulletHell(float dt, const Level&, const Player& player, Game& game) {
    if (type != BossType::BULLET_HELL || !active || !activated) {
        return;
    }

    float px = fixedToFloat(player.position.x) + fixedToFloat(Player::WIDTH) * 0.5f;
    float py = fixedToFloat(player.position.y) + fixedToFloat(Player::HEIGHT) * 0.5f;
    float ox = fixedToFloat(position.x) + fixedToFloat(WIDTH) * 0.5f;
    float oy = fixedToFloat(position.y) + fixedToFloat(HEIGHT) * 0.5f;

    auto fireTriple = [&]() {
        float base = std::atan2(py - oy, px - ox);
        float spread = BossEnemy::kBulletHellSpreadRadians;
        float angles[3] = {base - spread, base, base + spread};
        fixed_t halfMain = TO_FIXED(8.0f);
        fixed_t halfBossW = FIXED_DIV(WIDTH, TO_FIXED(2.0f));
        fixed_t halfBossH = FIXED_DIV(HEIGHT, TO_FIXED(2.0f));
        Vec2 origin(position.x + halfBossW - halfMain, position.y + halfBossH - halfMain);
        for (int i = 0; i < 3; ++i) {
            float c = std::cos(angles[i]);
            float s = std::sin(angles[i]);
            game.spawnProjectile(origin, Vec2(floatToFixed(c), floatToFixed(s)), kBulletHellShrapnelCount);
        }
    };

    switch (bulletHellState) {
        case BulletHellState::ProjectilePhase:
            // Fire three spreading shots once, then pause before charges.
            fireTriple();
            bulletHellState = BulletHellState::BetweenPhases;
            bulletHellStateTimer = 3.0f;
            break;

        case BulletHellState::BetweenPhases:
            bulletHellStateTimer -= dt;
            if (bulletHellStateTimer <= 0.0f) {
                bulletHellState = BulletHellState::ChargePhase;
                bulletHellChargeSub = BulletHellChargeSub::Warning;
                bulletHellStateTimer = 1.0f;
                chargeAttempts = 0;
            }
            break;

        case BulletHellState::ChargePhase:
            if (bulletHellChargeSub == BulletHellChargeSub::Warning) {
                bulletHellStateTimer -= dt;
                Vec2 bossCenter(floatToFixed(ox), floatToFixed(oy));
                Vec2 plCenter(floatToFixed(px), floatToFixed(py));
                game.showChargeWarning(bossCenter, plCenter);
                if (bulletHellStateTimer <= 0.0f) {
                    dashLockPlayerX = px;
                    dashLockPlayerY = py;
                    float dx = dashLockPlayerX - ox;
                    float dy = dashLockPlayerY - oy;
                    float len = std::sqrt(dx * dx + dy * dy);
                    if (len < 1e-4f) {
                        chargeDirX = 0.0f;
                        chargeDirY = 1.0f;
                        dashTotalDistance = kBulletHellDashPastPlayerPx;
                    } else {
                        chargeDirX = dx / len;
                        chargeDirY = dy / len;
                        dashTotalDistance = len + kBulletHellDashPastPlayerPx;
                    }
                    dashDistanceTraveled = 0.0f;
                    bulletHellChargeSub = BulletHellChargeSub::Dash;
                }
            } else {
                float step = kBulletHellChargeSpeedPx * dt;
                float remain = dashTotalDistance - dashDistanceTraveled;
                float move = std::min(step, remain);
                position.x = floatToFixed(fixedToFloat(position.x) + chargeDirX * move);
                position.y = floatToFixed(fixedToFloat(position.y) + chargeDirY * move);
                clampBossCenterToBulletHellArena(*this);
                dashDistanceTraveled += move;
                if (dashDistanceTraveled >= dashTotalDistance - 1e-2f) {
                    chargeAttempts += 1;
                    if (chargeAttempts >= 3) {
                        bulletHellState = BulletHellState::VulnerablePhase;
                        bulletHellStateTimer = 5.0f;
                    } else {
                        bulletHellChargeSub = BulletHellChargeSub::Warning;
                        bulletHellStateTimer = 1.0f;
                    }
                }
            }
            break;

        case BulletHellState::VulnerablePhase:
            bulletHellStateTimer -= dt;
            if (bulletHellStateTimer <= 0.0f) {
                bulletHellState = BulletHellState::ProjectilePhase;
            }
            break;
    }
}

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
    } else if (type == BossType::BULLET_HELL) {
        cx = bulletHellArenaCenterX;
        cy = bulletHellArenaCenterY;
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
    } else if (type == BossType::BULLET_HELL) {
        halfW = TO_FIXED(BULLET_HELL_ARENA_HALF_W_PX);
        halfH = TO_FIXED(BULLET_HELL_ARENA_HALF_H_PX);
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
    } else if (type == BossType::BULLET_HELL) {
        cx = bulletHellArenaCenterX;
        cy = bulletHellArenaCenterY;
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
    } else if (type == BossType::BULLET_HELL) {
        halfW = BULLET_HELL_ARENA_HALF_W_PX;
        halfH = BULLET_HELL_ARENA_HALF_H_PX;
    }
    float w2 = halfW * 2.0f;
    float h2 = halfH * 2.0f;

    out[0] = Rect(cx - halfW - t, cy - halfH, t, h2);
    out[1] = Rect(cx + halfW, cy - halfH, t, h2);
    out[2] = Rect(cx - halfW, cy - halfH - t, w2, t);
    out[3] = Rect(cx - halfW, cy + halfH, w2, t);
#endif
}
