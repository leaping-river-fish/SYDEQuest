#include "Level.h"
#include "../objective/Objective.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>

#ifdef PLATFORM_PICO
    #include "LevelMetadata.h"
#endif

Level::Level() : width(0), height(0), spawnPoint(100, 100) {
#ifdef PLATFORM_PICO
    portalCount = 0;
    basicEnemySpawnCount = 0;
    rangedEnemySpawnCount = 0;
    healthPackSpawnCount = 0;
    objectiveSpawnCount = 0;
#else
    tileIds = nullptr;
#endif
    loadTestLevel();
}

Level::~Level() {
    unload();
}

void Level::unload() {
#ifdef PLATFORM_PICO
    portalCount = 0;
    basicEnemySpawnCount = 0;
    rangedEnemySpawnCount = 0;
    healthPackSpawnCount = 0;
    objectiveSpawnCount = 0;
#else
    if (tileIds != nullptr) {
        delete[] tileIds;
        tileIds = nullptr;
    }
    portals.clear();
    enemySpawns.clear();
    basicEnemySpawns.clear();
    rangedEnemySpawns.clear();
    healthPackSpawns.clear();
    objectiveSpawns.clear();
#endif
    width = 0;
    height = 0;
}

int8_t Level::getTileId(int tileX, int tileY) const {
    if (tileX < 0 || tileX >= width || tileY < 0 || tileY >= height) {
        return 0;
    }
#ifdef PLATFORM_PICO
    return tileIds[tileY * width + tileX];
#else
    if (tileIds == nullptr) {
        return 0;
    }
    return tileIds[tileY * width + tileX];
#endif
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
    
#ifdef PLATFORM_PICO
    if (width * height > 128 * 64) {
        fclose(file);
        return false;
    }
    for (int i = 0; i < width * height; i++) {
        tileIds[i] = -1;
    }
#else
    tileIds = new int8_t[width * height];
    for (int i = 0; i < width * height; i++) {
        tileIds[i] = -1;
    }
#endif
    
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
#ifdef PLATFORM_PICO
                if (portalCount < 8) {
                    portals[portalCount++] = portal;
                }
#else
                portals.push_back(portal);
#endif
            }
        } else if (strncmp(line, "ENEMY_BASIC ", 12) == 0) {
            float ex, ey;
            if (sscanf(line + 12, "%f %f", &ex, &ey) == 2) {
#ifdef PLATFORM_PICO
                if (basicEnemySpawnCount < 25) {
                    basicEnemySpawns[basicEnemySpawnCount++] = Vec2(ex, ey);
                }
#else
                basicEnemySpawns.push_back(Vec2(ex, ey));
#endif
            }
        } else if (strncmp(line, "ENEMY_RANGED ", 13) == 0) {
            float ex, ey;
            if (sscanf(line + 13, "%f %f", &ex, &ey) == 2) {
#ifdef PLATFORM_PICO
                if (rangedEnemySpawnCount < 20) {
                    rangedEnemySpawns[rangedEnemySpawnCount++] = Vec2(ex, ey);
                }
#else
                rangedEnemySpawns.push_back(Vec2(ex, ey));
#endif
            }
        } else if (strncmp(line, "ENEMY ", 6) == 0) {
            float ex, ey;
            if (sscanf(line + 6, "%f %f", &ex, &ey) == 2) {
#ifdef PLATFORM_PICO
                if (basicEnemySpawnCount < 25) {
                    basicEnemySpawns[basicEnemySpawnCount++] = Vec2(ex, ey);
                }
#else
                enemySpawns.push_back(Vec2(ex, ey));
                basicEnemySpawns.push_back(Vec2(ex, ey));
#endif
            }
        } else if (strncmp(line, "HEALTH_PACK ", 12) == 0) {
            float hx, hy;
            if (sscanf(line + 12, "%f %f", &hx, &hy) == 2) {
#ifdef PLATFORM_PICO
                if (healthPackSpawnCount < 1) {
                    healthPackSpawns[healthPackSpawnCount++] = Vec2(hx, hy);
                }
#else
                healthPackSpawns.push_back(Vec2(hx, hy));
#endif
            }
        } else if (strncmp(line, "OBJECTIVE ", 10) == 0) {
            float ox, oy;
            char typeStr[32];
            if (sscanf(line + 10, "%f %f %s", &ox, &oy, typeStr) == 3) {
                ObjectiveType type;
                if (strcmp(typeStr, "CHARGER") == 0) {
                    type = ObjectiveType::CHARGER;
                } else if (strcmp(typeStr, "ENCLOSURE") == 0) {
                    type = ObjectiveType::ENCLOSURE;
                } else if (strcmp(typeStr, "HAPTIC") == 0) {
                    type = ObjectiveType::HAPTIC;
                } else if (strcmp(typeStr, "PARTS") == 0) {
                    type = ObjectiveType::PARTS;
                } else if (strcmp(typeStr, "SCREEN") == 0) {
                    type = ObjectiveType::SCREEN;
                } else if (strcmp(typeStr, "PICO") == 0) {
                    type = ObjectiveType::PICO;
                } else {
                    continue;
                }
#ifdef PLATFORM_PICO
                if (objectiveSpawnCount < 1) {
                    objectiveSpawns[objectiveSpawnCount] = Vec2(ox, oy);
                    objectiveTypes[objectiveSpawnCount] = type;
                    objectiveSpawnCount++;
                }
#else
                objectiveSpawns.push_back(std::make_pair(Vec2(ox, oy), type));
#endif
            }
        }
    }
    
    fclose(file);
    return true;
}

#ifdef PLATFORM_PICO
bool Level::loadFromData(const char* csvData) {
    if (!csvData) return false;
    
    unload();
    
    const char* ptr = csvData;
    int newWidth = 0;
    int newHeight = 0;
    
    const char* lineStart = ptr;
    while (*ptr && *ptr != '\n') {
        if (*ptr == ',') newWidth++;
        ptr++;
    }
    newWidth++;
    
    ptr = csvData;
    while (*ptr) {
        while (*ptr && *ptr != '\n') ptr++;
        if (*ptr == '\n') {
            ptr++;
            if (*ptr == '\0' || (*ptr >= 'A' && *ptr <= 'Z') || (*ptr >= 'a' && *ptr <= 'z')) {
                break;
            }
            newHeight++;
        }
    }
    
    if (newWidth <= 0 || newHeight <= 0 || newWidth * newHeight > 128 * 64) {
        return false;
    }
    
    width = newWidth;
    height = newHeight;
    
    for (int i = 0; i < width * height; i++) {
        tileIds[i] = -1;
    }
    
    ptr = csvData;
    for (int y = 0; y < height; y++) {
        int x = 0;
        while (*ptr && *ptr != '\n' && x < width) {
            if (*ptr == ',' || *ptr == '-' || (*ptr >= '0' && *ptr <= '9')) {
                int value = 0;
                bool negative = false;
                if (*ptr == '-') {
                    negative = true;
                    ptr++;
                }
                while (*ptr >= '0' && *ptr <= '9') {
                    value = value * 10 + (*ptr - '0');
                    ptr++;
                }
                tileIds[y * width + x] = negative ? -value : value;
                x++;
                if (*ptr == ',') ptr++;
            } else {
                ptr++;
            }
        }
        while (*ptr && *ptr != '\n') ptr++;
        if (*ptr == '\n') ptr++;
    }
    
    while (*ptr) {
        lineStart = ptr;
        while (*ptr && *ptr != '\n') ptr++;
        int lineLen = ptr - lineStart;
        
        if (lineLen > 0) {
            char line[256];
            int copyLen = lineLen < 255 ? lineLen : 255;
            for (int i = 0; i < copyLen; i++) {
                line[i] = lineStart[i];
            }
            line[copyLen] = '\0';
            
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
                    if (portalCount < 8) {
                        portals[portalCount++] = portal;
                    }
                }
            } else if (strncmp(line, "ENEMY_BASIC ", 12) == 0) {
                float ex, ey;
                if (sscanf(line + 12, "%f %f", &ex, &ey) == 2) {
                    if (basicEnemySpawnCount < 25) {
                        basicEnemySpawns[basicEnemySpawnCount++] = Vec2(ex, ey);
                    }
                }
            } else if (strncmp(line, "ENEMY_RANGED ", 13) == 0) {
                float ex, ey;
                if (sscanf(line + 13, "%f %f", &ex, &ey) == 2) {
                    if (rangedEnemySpawnCount < 20) {
                        rangedEnemySpawns[rangedEnemySpawnCount++] = Vec2(ex, ey);
                    }
                }
            } else if (strncmp(line, "ENEMY ", 6) == 0) {
                float ex, ey;
                if (sscanf(line + 6, "%f %f", &ex, &ey) == 2) {
                    if (basicEnemySpawnCount < 25) {
                        basicEnemySpawns[basicEnemySpawnCount++] = Vec2(ex, ey);
                    }
                }
            } else if (strncmp(line, "HEALTH_PACK ", 12) == 0) {
                float hx, hy;
                if (sscanf(line + 12, "%f %f", &hx, &hy) == 2) {
                    if (healthPackSpawnCount < 1) {
                        healthPackSpawns[healthPackSpawnCount++] = Vec2(hx, hy);
                    }
                }
            } else if (strncmp(line, "OBJECTIVE ", 10) == 0) {
                float ox, oy;
                char typeStr[32];
                if (sscanf(line + 10, "%f %f %s", &ox, &oy, typeStr) == 3) {
                    ObjectiveType type;
                    if (strcmp(typeStr, "CHARGER") == 0) {
                        type = ObjectiveType::CHARGER;
                    } else if (strcmp(typeStr, "ENCLOSURE") == 0) {
                        type = ObjectiveType::ENCLOSURE;
                    } else if (strcmp(typeStr, "HAPTIC") == 0) {
                        type = ObjectiveType::HAPTIC;
                    } else if (strcmp(typeStr, "PARTS") == 0) {
                        type = ObjectiveType::PARTS;
                    } else if (strcmp(typeStr, "SCREEN") == 0) {
                        type = ObjectiveType::SCREEN;
                    } else if (strcmp(typeStr, "PICO") == 0) {
                        type = ObjectiveType::PICO;
                    } else {
                        continue;
                    }
                    if (objectiveSpawnCount < 1) {
                        objectiveSpawns[objectiveSpawnCount] = Vec2(ox, oy);
                        objectiveTypes[objectiveSpawnCount] = type;
                        objectiveSpawnCount++;
                    }
                }
            }
        }
        
        if (*ptr == '\n') ptr++;
    }
    
    return true;
}

bool Level::loadFromBinaryData(const int8_t* tiles, int widthIn, int heightIn,
                               const LevelMetadata* metadata) {
    if (!tiles || !metadata) return false;
    if (widthIn <= 0 || heightIn <= 0 || widthIn * heightIn > 128 * 64) return false;

    unload();

    width = widthIn;
    height = heightIn;
    std::memcpy(tileIds, tiles, static_cast<size_t>(width * height) * sizeof(int8_t));

    spawnPoint = Vec2(metadata->spawn.x, metadata->spawn.y);
    
    for (uint8_t i = 0; i < metadata->enemyCount && i < (25 + 20); i++) {
        const EnemySpawnData& enemy = metadata->enemies[i];
        if (enemy.type == LevelEnemyType::BASIC) {
            if (basicEnemySpawnCount < 25) {
                basicEnemySpawns[basicEnemySpawnCount++] = Vec2(enemy.x, enemy.y);
            }
        } else if (enemy.type == LevelEnemyType::RANGED) {
            if (rangedEnemySpawnCount < 20) {
                rangedEnemySpawns[rangedEnemySpawnCount++] = Vec2(enemy.x, enemy.y);
            }
        }
    }
    
    for (uint8_t i = 0; i < metadata->healthPackCount && i < 1; i++) {
        healthPackSpawns[healthPackSpawnCount++] = Vec2(
            metadata->healthPacks[i].x,
            metadata->healthPacks[i].y
        );
    }
    
    for (uint8_t i = 0; i < metadata->portalCount && i < 8; i++) {
        const PortalData& pd = metadata->portals[i];
        Portal portal;
        portal.bounds = Rect(pd.x, pd.y, pd.w, pd.h);
        
        if (pd.targetLevelId == 0) {
            strncpy(portal.targetLevel, "../levels/Level1.csv", sizeof(portal.targetLevel) - 1);
        } else if (pd.targetLevelId == 1) {
            strncpy(portal.targetLevel, "../levels/Level2.csv", sizeof(portal.targetLevel) - 1);
        }
        portal.targetLevel[sizeof(portal.targetLevel) - 1] = '\0';
        
        portal.targetSpawn = Vec2(pd.targetX, pd.targetY);
        portals[portalCount++] = portal;
    }
    
    for (uint8_t i = 0; i < metadata->objectiveCount && i < 1; i++) {
        const ObjectiveSpawnData& obj = metadata->objectives[i];
        objectiveSpawns[objectiveSpawnCount] = Vec2(obj.x, obj.y);
        
        ObjectiveType type;
        switch (obj.type) {
            case LevelObjectiveType::CHARGER: type = ObjectiveType::CHARGER; break;
            case LevelObjectiveType::ENCLOSURE: type = ObjectiveType::ENCLOSURE; break;
            case LevelObjectiveType::HAPTIC: type = ObjectiveType::HAPTIC; break;
            case LevelObjectiveType::PARTS: type = ObjectiveType::PARTS; break;
            case LevelObjectiveType::SCREEN: type = ObjectiveType::SCREEN; break;
            case LevelObjectiveType::PICO: type = ObjectiveType::PICO; break;
            default: type = ObjectiveType::PICO; break;
        }
        objectiveTypes[objectiveSpawnCount] = type;
        objectiveSpawnCount++;
    }
    
    return true;
}
#endif

void Level::loadTestLevel() {
    unload();
    
    width = 64;
    height = 32;
    
#ifdef PLATFORM_PICO
    for (int i = 0; i < width * height; i++) {
        tileIds[i] = -1;
    }
#else
    tileIds = new int8_t[width * height];
    for (int i = 0; i < width * height; i++) {
        tileIds[i] = -1;
    }
#endif
    
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
    
    spawnPoint = Vec2(100.0f, 100.0f);
}
