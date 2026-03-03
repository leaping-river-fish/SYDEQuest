#include "Level.h"

Level::Level() {
    // initialize all tiles to air
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            tiles[y][x] = TileType::Air;
        }
    }

    loadTestLevel();
}

TileType Level::getTile(int tileX, int tileY) const {
    if (tileX < 0 || tileX >= WIDTH || tileY < 0 || tileY >= HEIGHT) {
        return TileType::Solid; // out of bounds tiles are solid
    }
    return tiles[tileY][tileX];
}

bool Level::isSolid(int tileX, int tileY) const {
    TileType tile = getTile(tileX, tileY);
    return tile == TileType::Solid;
}

bool Level::isPlatform(int tileX, int tileY) const {
    return getTile(tileX, tileY) == TileType::Platform;
}

// TODO: Replace with actual level loading from file
void Level::loadTestLevel() {
    // Floor
    for (int x = 0; x < WIDTH; x++) {
        tiles[HEIGHT - 1][x] = TileType::Solid;
        tiles[HEIGHT - 2][x] = TileType::Solid;
    }
    
    // Left wall
    for (int y = 0; y < HEIGHT; y++) {
        tiles[y][0] = TileType::Solid;
    }
    
    // Right wall
    for (int y = 0; y < HEIGHT; y++) {
        tiles[y][WIDTH - 1] = TileType::Solid;
    }
    
    // Some platforms (all one-way)
    for (int x = 10; x < 20; x++) {
        tiles[HEIGHT - 6][x] = TileType::Platform;
    }
    
    for (int x = 25; x < 35; x++) {
        tiles[HEIGHT - 10][x] = TileType::Platform;
    }

    for (int x = 50; x < 60; x++) {
        tiles[HEIGHT - 6][x] = TileType::Platform;
    }
    
    // Staircase (one-way platforms)
    for (int i = 0; i < 5; i++) {
        tiles[HEIGHT - 3 - i][40 + i] = TileType::Platform;
    }
}
