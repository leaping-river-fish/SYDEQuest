#pragma once
#include "../../core/types.h"

enum class LevelEnemyType : uint8_t {
    BASIC = 0,
    RANGED = 1
};

enum class LevelObjectiveType : uint8_t {
    CHARGER = 0,
    ENCLOSURE = 1,
    HAPTIC = 2,
    PARTS = 3,
    SCREEN = 4,
    PICO = 5
};

enum class LevelBossType : uint8_t {
    SUMMONER = 0,
    LASER = 1,
    BULLET_HELL = 2
};

struct SpawnData {
    float x;
    float y;
};

struct EnemySpawnData {
    float x;
    float y;
    LevelEnemyType type;
};

struct PortalData {
    float x;
    float y;
    float w;
    float h;
    uint8_t targetLevelId;
    float targetX;
    float targetY;
};

struct ObjectiveSpawnData {
    float x;
    float y;
    LevelObjectiveType type;
};

struct BossSpawnData {
    float x;
    float y;
    LevelBossType type;
};

struct LevelMetadata {
    SpawnData spawn;
    
    const EnemySpawnData* enemies;
    uint8_t enemyCount;
    
    const SpawnData* healthPacks;
    uint8_t healthPackCount;
    
    const PortalData* portals;
    uint8_t portalCount;
    
    const ObjectiveSpawnData* objectives;
    uint8_t objectiveCount;
    
    const BossSpawnData* bosses;
    uint8_t bossCount;
};
