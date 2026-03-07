#include "Level.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>

Level::Level() : width(0), height(0), tileIds(nullptr), spawnPoint(100, 100) {
    loadTestLevel();
}

Level::~Level() {
    unload();
}

void Level::unload() {
    if (tileIds != nullptr) {
        delete[] tileIds;
        tileIds = nullptr;
    }
    width = 0;
    height = 0;
    portals.clear();
    enemySpawns.clear();
    basicEnemySpawns.clear();
    rangedEnemySpawns.clear();
}

int8_t Level::getTileId(int tileX, int tileY) const {
    if (tileIds == nullptr || tileX < 0 || tileX >= width || tileY < 0 || tileY >= height) {
        return 0;  // Return solid tile for out of bounds
    }
    return tileIds[tileY * width + tileX];
}

bool Level::isSolid(int tileX, int tileY) const {
    int8_t tileId = getTileId(tileX, tileY);
    return tileId >= 0 && tileId <= 11;  // Tile IDs 0-11 are solid blocks
}

bool Level::isPlatform(int tileX, int tileY) const {
    int8_t tileId = getTileId(tileX, tileY);
    return tileId >= 12 && tileId <= 14;  // Tile IDs 12-14 are platforms
}

bool Level::loadFromFile(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        return false;
    }
    
    // First pass: determine dimensions by reading the file
    char line[4096];
    int newWidth = 0;
    int newHeight = 0;
    
    // Read first line to get width
    if (fgets(line, sizeof(line), file)) {
        // Count commas + 1 to get width
        newWidth = 1;
        for (int i = 0; line[i] != '\0' && line[i] != '\n'; i++) {
            if (line[i] == ',') {
                newWidth++;
            }
        }
        newHeight = 1;
        
        // Count remaining lines (stop at metadata lines)
        while (fgets(line, sizeof(line), file)) {
            // Skip empty lines
            if (line[0] == '\n' || line[0] == '\0') {
                continue;
            }
            // Stop counting if we hit a metadata line (starts with a letter)
            if ((line[0] >= 'A' && line[0] <= 'Z') || (line[0] >= 'a' && line[0] <= 'z')) {
                break;
            }
            newHeight++;
        }
    }
    
    if (newWidth <= 0 || newHeight <= 0 || newWidth > 1024 || newHeight > 1024) {
        fclose(file);
        return false;
    }
    
    // Reset file to beginning
    fseek(file, 0, SEEK_SET);
    
    unload();
    
    width = newWidth;
    height = newHeight;
    tileIds = new int8_t[width * height];
    
    // Initialize all tiles to air
    for (int i = 0; i < width * height; i++) {
        tileIds[i] = -1;
    }
    
    // Second pass: parse the CSV data
    for (int y = 0; y < height; y++) {
        if (!fgets(line, sizeof(line), file)) {
            fclose(file);
            unload();
            return false;
        }
        
        // Parse comma-separated values
        int x = 0;
        char* token = strtok(line, ",");
        while (token != nullptr && x < width) {
            int tileValue = atoi(token);
            if (tileValue >= -1 && tileValue <= 14) {
                tileIds[y * width + x] = static_cast<int8_t>(tileValue);
            }
            x++;
            token = strtok(nullptr, ",");
        }
    }
    
    // Parse metadata (SPAWN, PORTAL, ENEMY, etc.)
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
        } else if (strncmp(line, "ENEMY_BASIC ", 12) == 0) {
            float ex, ey;
            if (sscanf(line + 12, "%f %f", &ex, &ey) == 2) {
                basicEnemySpawns.push_back(Vec2(ex, ey));
            }
        } else if (strncmp(line, "ENEMY_RANGED ", 13) == 0) {
            float ex, ey;
            if (sscanf(line + 13, "%f %f", &ex, &ey) == 2) {
                rangedEnemySpawns.push_back(Vec2(ex, ey));
            }
        } else if (strncmp(line, "ENEMY ", 6) == 0) {
            // Legacy support: ENEMY defaults to basic enemy
            float ex, ey;
            if (sscanf(line + 6, "%f %f", &ex, &ey) == 2) {
                enemySpawns.push_back(Vec2(ex, ey));
                basicEnemySpawns.push_back(Vec2(ex, ey));
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
    tileIds = new int8_t[width * height];
    
    // Initialize all tiles to air (-1)
    for (int i = 0; i < width * height; i++) {
        tileIds[i] = -1;
    }
    
    // Bottom two rows - solid blocks (using tile ID 4 for middle fill)
    for (int x = 0; x < width; x++) {
        tileIds[(height - 1) * width + x] = 4;
        tileIds[(height - 2) * width + x] = 4;
    }
    
    // Left wall - solid blocks
    for (int y = 0; y < height; y++) {
        tileIds[y * width + 0] = 4;
    }
    
    // Right wall - solid blocks
    for (int y = 0; y < height; y++) {
        tileIds[y * width + (width - 1)] = 4;
    }
    
    // Platforms (using tile ID 13 for middle platform piece)
    for (int x = 10; x < 20; x++) {
        tileIds[(height - 6) * width + x] = 13;
    }
    
    for (int x = 25; x < 35; x++) {
        tileIds[(height - 10) * width + x] = 13;
    }

    for (int x = 50; x < 60; x++) {
        tileIds[(height - 6) * width + x] = 13;
    }
    
    for (int i = 0; i < 5; i++) {
        tileIds[(height - 3 - i) * width + (40 + i)] = 13;
    }
    
    spawnPoint = Vec2(100, 100);
}
