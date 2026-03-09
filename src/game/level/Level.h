#pragma once
#include <cstdint>
#include <vector>
#include "../../core/types.h"

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
    const std::vector<Portal>& getPortals() const { return portals; }
    const std::vector<Vec2>& getEnemySpawns() const { return enemySpawns; }
    const std::vector<Vec2>& getBasicEnemySpawns() const { return basicEnemySpawns; }
    const std::vector<Vec2>& getRangedEnemySpawns() const { return rangedEnemySpawns; }
    const std::vector<Vec2>& getHealthPackSpawns() const { return healthPackSpawns; }

private:
    int width;
    int height;
    int8_t* tileIds;  // Stores tile IDs from Tiled (-1 to 14)
    Vec2 spawnPoint;
    std::vector<Portal> portals;
    std::vector<Vec2> enemySpawns;  // Legacy: maps to basicEnemySpawns
    std::vector<Vec2> basicEnemySpawns;
    std::vector<Vec2> rangedEnemySpawns;
    std::vector<Vec2> healthPackSpawns;

    void loadTestLevel();
};