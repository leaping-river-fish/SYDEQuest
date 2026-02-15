#pragma once
#include <cstdint>

enum class TileType : uint8_t {
    Air = 0,
    Solid = 1,
    Platform = 2, // One-way platform
};

class Level {
public:
    static constexpr int WIDTH = 64;
    static constexpr int HEIGHT = 32;
    static constexpr int TILE_SIZE = 16;

    Level();

    TileType getTile(int tileX, int tileY) const;
    bool isSolid(int tileX, int tileY) const;
    bool isPlatform(int tileX, int tileY) const;

    int getWidthInTiles() const { return WIDTH; }
    int getHeightInTiles() const { return HEIGHT; }
    int getTileSize() const { return TILE_SIZE; }
    int getWidthInPixels() const { return WIDTH * TILE_SIZE; }
    int getHeightInPixels() const { return HEIGHT * TILE_SIZE; }

private:
    TileType tiles[HEIGHT][WIDTH];

    void loadTestLevel();
};