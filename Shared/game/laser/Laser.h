#pragma once

#include "../../core/types.h"

class IRenderer;
class Level;

struct Laser {
    static constexpr int MAX_BOUNCES = 4;
    static constexpr int MAX_POINTS = MAX_BOUNCES + 2;

    Vec2 points[MAX_POINTS];
    uint8_t pointCount;
    float age;
    float telegraphDuration;
    float totalLifetime;
    bool active;

    Laser();
};

/** World-space path with axis-aligned bounces off tile solids and arena inner bounds. */
bool buildLaserPath(Vec2 origin, float dirX, float dirY, const Level& level, float arenaCx, float arenaCy,
                    float arenaHalfW, float arenaHalfH, int maxBounces, Vec2* outPoints, int& outPointCount);

float distanceSqPointToSegment(const Vec2& p, const Vec2& a, const Vec2& b);

void renderLaser(const Laser& laser, IRenderer* renderer, int cameraX, int cameraY);
