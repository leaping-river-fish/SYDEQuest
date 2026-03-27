#pragma once
#include <cstdint>
#include "../../core/types.h"

#ifndef PLATFORM_PICO
    #include <vector>
    #include <utility>
#endif

// Forward declare ObjectiveType from Objective.h
enum class ObjectiveType;

#ifdef PLATFORM_PICO
// Forward declare LevelMetadata from LevelMetadata.h
struct LevelMetadata;
#endif

// Tile IDs from Tiled map editor:
// -1: Air (empty space)
// 0-11: Solid blocks (various corners, edges, fills)
// 12-14: Platform pieces (left, middle, right)

struct Portal {
    Rect bounds;
    char targetLevel[64];
    Vec2 targetSpawn;
    
    Portal() : bounds(), targetSpawn() {
        targetLevel[0] = '\0';
    }
};

class Level {
public:
    static constexpr int TILE_SIZE = 16;
    static constexpr int PLATFORM_HEIGHT = 8;

    Level();
    ~Level();

    bool loadFromFile(const char* filename);
#ifdef PLATFORM_PICO
    bool loadFromData(const char* csvData);
    bool loadFromBinaryData(const int8_t* tiles, int width, int height, const LevelMetadata* metadata);
#endif
    void unload();
    
    int8_t getTileId(int tileX, int tileY) const;
    bool isSolid(int tileX, int tileY) const;
    bool isPlatform(int tileX, int tileY) const;

    int getWidthInTiles() const { return width; }
    int getHeightInTiles() const { return height; }
    int getTileSize() const { return TILE_SIZE; }
    int getWidthInPixels() const { return width * TILE_SIZE; }
    int getHeightInPixels() const { return height * TILE_SIZE; }
    
    Vec2 getSpawnPoint() const { return spawnPoint; }
    
#ifdef PLATFORM_PICO
    const Portal* getPortals() const { return portals; }
    uint8_t getPortalCount() const { return portalCount; }
    const Vec2* getBasicEnemySpawns() const { return basicEnemySpawns; }
    uint8_t getBasicEnemySpawnCount() const { return basicEnemySpawnCount; }
    const Vec2* getRangedEnemySpawns() const { return rangedEnemySpawns; }
    uint8_t getRangedEnemySpawnCount() const { return rangedEnemySpawnCount; }
    const Vec2* getHealthPackSpawns() const { return healthPackSpawns; }
    uint8_t getHealthPackSpawnCount() const { return healthPackSpawnCount; }
    const Vec2* getObjectiveSpawns() const { return objectiveSpawns; }
    const ObjectiveType* getObjectiveTypes() const { return objectiveTypes; }
    uint8_t getObjectiveSpawnCount() const { return objectiveSpawnCount; }
#else
    const std::vector<Portal>& getPortals() const { return portals; }
    const std::vector<Vec2>& getEnemySpawns() const { return enemySpawns; }
    const std::vector<Vec2>& getBasicEnemySpawns() const { return basicEnemySpawns; }
    const std::vector<Vec2>& getRangedEnemySpawns() const { return rangedEnemySpawns; }
    const std::vector<Vec2>& getHealthPackSpawns() const { return healthPackSpawns; }
    const std::vector<std::pair<Vec2, ObjectiveType>>& getObjectiveSpawns() const { return objectiveSpawns; }
#endif

private:
    int width;
    int height;
    Vec2 spawnPoint;
    
#ifdef PLATFORM_PICO
    int8_t tileIds[128 * 64];
    Portal portals[1];
    uint8_t portalCount;
    Vec2 basicEnemySpawns[10];
    uint8_t basicEnemySpawnCount;
    Vec2 rangedEnemySpawns[20];
    uint8_t rangedEnemySpawnCount;
    Vec2 healthPackSpawns[1];
    uint8_t healthPackSpawnCount;
    Vec2 objectiveSpawns[1];
    uint8_t objectiveSpawnCount;
    ObjectiveType objectiveTypes[1];
#else
    int8_t* tileIds;
    std::vector<Portal> portals;
    std::vector<Vec2> enemySpawns;
    std::vector<Vec2> basicEnemySpawns;
    std::vector<Vec2> rangedEnemySpawns;
    std::vector<Vec2> healthPackSpawns;
    std::vector<std::pair<Vec2, ObjectiveType>> objectiveSpawns;
#endif

    void loadTestLevel();
};