#pragma once
#include <cstdint>
#include <vector>
#include "../../core/types.h"

enum class TileType : uint8_t {
    Air = 0,
    Solid = 1,
    Platform = 2,
};

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
    
    TileType getTile(int tileX, int tileY) const;
    bool isSolid(int tileX, int tileY) const;
    bool isPlatform(int tileX, int tileY) const;

    int getWidthInTiles() const { return width; }
    int getHeightInTiles() const { return height; }
    int getTileSize() const { return TILE_SIZE; }
    int getWidthInPixels() const { return width * TILE_SIZE; }
    int getHeightInPixels() const { return height * TILE_SIZE; }
    
    Vec2 getSpawnPoint() const { return spawnPoint; }
    const std::vector<Portal>& getPortals() const { return portals; }

private:
    int width;
    int height;
    TileType* tiles;
    Vec2 spawnPoint;
    std::vector<Portal> portals;

    void loadTestLevel();
};