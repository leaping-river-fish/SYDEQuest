#include "Level.h"
#include <cstdio>
#include <cstring>

Level::Level() : width(0), height(0), tiles(nullptr), spawnPoint(100, 100) {
    loadTestLevel();
}

Level::~Level() {
    unload();
}

void Level::unload() {
    if (tiles != nullptr) {
        delete[] tiles;
        tiles = nullptr;
    }
    width = 0;
    height = 0;
    portals.clear();
}

TileType Level::getTile(int tileX, int tileY) const {
    if (tiles == nullptr || tileX < 0 || tileX >= width || tileY < 0 || tileY >= height) {
        return TileType::Solid;
    }
    return tiles[tileY * width + tileX];
}

bool Level::isSolid(int tileX, int tileY) const {
    TileType tile = getTile(tileX, tileY);
    return tile == TileType::Solid;
}

bool Level::isPlatform(int tileX, int tileY) const {
    return getTile(tileX, tileY) == TileType::Platform;
}

bool Level::loadFromFile(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        return false;
    }
    
    int newWidth, newHeight;
    if (fscanf(file, "%d %d\n", &newWidth, &newHeight) != 2) {
        fclose(file);
        return false;
    }
    
    if (newWidth <= 0 || newHeight <= 0 || newWidth > 1024 || newHeight > 1024) {
        fclose(file);
        return false;
    }
    
    unload();
    
    width = newWidth;
    height = newHeight;
    tiles = new TileType[width * height];
    
    for (int i = 0; i < width * height; i++) {
        tiles[i] = TileType::Air;
    }
    
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int tileValue;
            if (fscanf(file, "%1d", &tileValue) != 1) {
                fclose(file);
                unload();
                return false;
            }
            
            if (tileValue >= 0 && tileValue <= 2) {
                tiles[y * width + x] = static_cast<TileType>(tileValue);
            }
        }
        fscanf(file, "\n");
    }
    
    char line[256];
    while (fgets(line, sizeof(line), file)) {
        if (strncmp(line, "SPAWN ", 6) == 0) {
            float sx, sy;
            if (sscanf(line + 6, "%f %f", &sx, &sy) == 2) {
                spawnPoint = Vec2(sx, sy);
            }
        } else if (strncmp(line, "PORTAL ", 7) == 0) {
            Portal portal;
            float px, py, pw, ph, tsx, tsy;
            char targetFile[64];
            if (sscanf(line + 7, "%f %f %f %f %s %f %f", 
                      &px, &py, &pw, &ph, targetFile, &tsx, &tsy) == 7) {
                portal.bounds = Rect(px, py, pw, ph);
                strncpy(portal.targetLevel, targetFile, sizeof(portal.targetLevel) - 1);
                portal.targetLevel[sizeof(portal.targetLevel) - 1] = '\0';
                portal.targetSpawn = Vec2(tsx, tsy);
                portals.push_back(portal);
            }
        }
    }
    
    fclose(file);
    return true;
}

void Level::loadTestLevel() {
    unload();
    
    width = 64;
    height = 32;
    tiles = new TileType[width * height];
    
    for (int i = 0; i < width * height; i++) {
        tiles[i] = TileType::Air;
    }
    
    for (int x = 0; x < width; x++) {
        tiles[(height - 1) * width + x] = TileType::Solid;
        tiles[(height - 2) * width + x] = TileType::Solid;
    }
    
    for (int y = 0; y < height; y++) {
        tiles[y * width + 0] = TileType::Solid;
    }
    
    for (int y = 0; y < height; y++) {
        tiles[y * width + (width - 1)] = TileType::Solid;
    }
    
    for (int x = 10; x < 20; x++) {
        tiles[(height - 6) * width + x] = TileType::Platform;
    }
    
    for (int x = 25; x < 35; x++) {
        tiles[(height - 10) * width + x] = TileType::Platform;
    }

    for (int x = 50; x < 60; x++) {
        tiles[(height - 6) * width + x] = TileType::Platform;
    }
    
    for (int i = 0; i < 5; i++) {
        tiles[(height - 3 - i) * width + (40 + i)] = TileType::Platform;
    }
    
    spawnPoint = Vec2(100, 100);
}
