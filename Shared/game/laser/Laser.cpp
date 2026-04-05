#include "Laser.h"

#include "../../core/IRenderer.h"
#include "../level/Level.h"

#include <algorithm>
#include <cmath>

namespace {

constexpr float kTileF = static_cast<float>(Level::TILE_SIZE);
constexpr float kEps = 1e-4f;
constexpr float kNudge = 0.05f;
constexpr float kTerminalExtend = 1800.0f;

inline void setPoint(Vec2* out, float x, float y) {
#ifdef PLATFORM_PICO
    *out = Vec2(x, y);
#else
    out->x = x;
    out->y = y;
#endif
}

bool insideAABB(float x, float y, float minX, float minY, float maxX, float maxY) {
    return x >= minX - kEps && x <= maxX + kEps && y >= minY - kEps && y <= maxY + kEps;
}

bool arenaExitInner(float ox, float oy, float ux, float uy, float minX, float minY, float maxX, float maxY,
                    float& outT, int& outFace) {
    float tMin = 1e30f;
    int face = -1;
    if (ux > kEps) {
        float t = (maxX - ox) / ux;
        float y = oy + t * uy;
        if (t > kEps && y >= minY - kEps && y <= maxY + kEps && t < tMin) {
            tMin = t;
            face = 1;
        }
    }
    if (ux < -kEps) {
        float t = (minX - ox) / ux;
        float y = oy + t * uy;
        if (t > kEps && y >= minY - kEps && y <= maxY + kEps && t < tMin) {
            tMin = t;
            face = 0;
        }
    }
    if (uy > kEps) {
        float t = (maxY - oy) / uy;
        float x = ox + t * ux;
        if (t > kEps && x >= minX - kEps && x <= maxX + kEps && t < tMin) {
            tMin = t;
            face = 3;
        }
    }
    if (uy < -kEps) {
        float t = (minY - oy) / uy;
        float x = ox + t * ux;
        if (t > kEps && x >= minX - kEps && x <= maxX + kEps && t < tMin) {
            tMin = t;
            face = 2;
        }
    }
    if (face < 0) {
        return false;
    }
    outT = tMin;
    outFace = face;
    return true;
}

bool tileRayFirstSolid(const Level& level, float ox, float oy, float ux, float uy, float maxT, float& outT,
                       bool& verticalEdge) {
    float gx = ox / kTileF;
    float gy = oy / kTileF;
    int mapX = static_cast<int>(std::floor(gx));
    int mapY = static_cast<int>(std::floor(gy));

    if (level.isSolid(mapX, mapY)) {
        outT = 0.f;
        verticalEdge = true;
        return true;
    }

    float deltaDistX = (std::abs(ux) < 1e-12f) ? 1e30f : std::abs(kTileF / ux);
    float deltaDistY = (std::abs(uy) < 1e-12f) ? 1e30f : std::abs(kTileF / uy);
    int stepX = (ux >= 0.f) ? 1 : -1;
    int stepY = (uy >= 0.f) ? 1 : -1;

    float sideDistX =
        (ux < 0.f) ? (gx - static_cast<float>(mapX)) * deltaDistX : (static_cast<float>(mapX) + 1.f - gx) * deltaDistX;
    float sideDistY =
        (uy < 0.f) ? (gy - static_cast<float>(mapY)) * deltaDistY : (static_cast<float>(mapY) + 1.f - gy) * deltaDistY;

    float totalT = 0.f;
    for (int iter = 0; iter < 1024; ++iter) {
        if (sideDistX < sideDistY) {
            float dt = sideDistX;
            totalT += dt;
            if (totalT > maxT + kEps) {
                return false;
            }
            sideDistY -= dt;
            sideDistX = deltaDistX;
            mapX += stepX;
            verticalEdge = true;
        } else {
            float dt = sideDistY;
            totalT += dt;
            if (totalT > maxT + kEps) {
                return false;
            }
            sideDistX -= dt;
            sideDistY = deltaDistY;
            mapY += stepY;
            verticalEdge = false;
        }
        if (level.isSolid(mapX, mapY)) {
            outT = totalT;
            return true;
        }
    }
    return false;
}

}  // namespace

Laser::Laser()
    : pointCount(0)
    , age(0.f)
    , telegraphDuration(1.5f)
    , totalLifetime(3.5f)  // 1.5s warning line + 2.0s damaging beam
    , active(false) {}

bool buildLaserPath(Vec2 origin, float dirX, float dirY, const Level& level, float arenaCx, float arenaCy,
                    float arenaHalfW, float arenaHalfH, int maxBounces, Vec2* outPoints, int& outPointCount) {
    float len = std::sqrt(dirX * dirX + dirY * dirY);
    if (len < 1e-8f) {
        return false;
    }
    float ux = dirX / len;
    float uy = dirY / len;

    float minX = arenaCx - arenaHalfW;
    float maxX = arenaCx + arenaHalfW;
    float minY = arenaCy - arenaHalfH;
    float maxY = arenaCy + arenaHalfH;

    float ox = fixedToFloat(origin.x);
    float oy = fixedToFloat(origin.y);

    outPointCount = 0;
    outPoints[outPointCount] = origin;
    outPointCount++;

    for (int b = 0; b < maxBounces; ++b) {
        float tArena = 1e30f;
        int arenaFace = 0;
        bool hasArena =
            insideAABB(ox, oy, minX, minY, maxX, maxY) && arenaExitInner(ox, oy, ux, uy, minX, minY, maxX, maxY, tArena, arenaFace);

        float maxTileT = hasArena ? tArena : 50000.f;
        float tTile = 1e30f;
        bool vertTile = false;
        bool hasTile = tileRayFirstSolid(level, ox, oy, ux, uy, maxTileT, tTile, vertTile);

        if (!hasTile && !hasArena) {
            break;
        }

        bool useTile = false;
        if (hasTile && hasArena) {
            useTile = (tTile < tArena - kEps);
        } else if (hasTile) {
            useTile = true;
        }

        float tHit = useTile ? tTile : tArena;
        if (tHit > 1e20f) {
            break;
        }

        float hx = ox + ux * tHit;
        float hy = oy + uy * tHit;
        setPoint(&outPoints[outPointCount], hx, hy);
        outPointCount++;
        if (outPointCount >= Laser::MAX_POINTS) {
            break;
        }

        if (useTile) {
            if (vertTile) {
                ux = -ux;
            } else {
                uy = -uy;
            }
        } else {
            if (arenaFace == 0 || arenaFace == 1) {
                ux = -ux;
            } else {
                uy = -uy;
            }
        }
        ox = hx + ux * kNudge;
        oy = hy + uy * kNudge;
    }

    float tEnd = kTerminalExtend;
    float tA = 1e30f;
    int face = 0;
    if (insideAABB(ox, oy, minX, minY, maxX, maxY) && arenaExitInner(ox, oy, ux, uy, minX, minY, maxX, maxY, tA, face)) {
        tEnd = std::min(tEnd, tA);
    }
    float tS = 1e30f;
    bool v = false;
    if (tileRayFirstSolid(level, ox, oy, ux, uy, tEnd, tS, v)) {
        tEnd = std::min(tEnd, tS);
    }
    float fx = ox + ux * tEnd;
    float fy = oy + uy * tEnd;
    if (outPointCount < Laser::MAX_POINTS) {
        setPoint(&outPoints[outPointCount], fx, fy);
        outPointCount++;
    }

    return outPointCount >= 2;
}

float distanceSqPointToSegment(const Vec2& p, const Vec2& a, const Vec2& b) {
    float px = fixedToFloat(p.x);
    float py = fixedToFloat(p.y);
    float ax = fixedToFloat(a.x);
    float ay = fixedToFloat(a.y);
    float bx = fixedToFloat(b.x);
    float by = fixedToFloat(b.y);
    float abx = bx - ax;
    float aby = by - ay;
    float apx = px - ax;
    float apy = py - ay;
    float abLenSq = abx * abx + aby * aby;
    if (abLenSq < 1e-12f) {
        float dx = px - ax;
        float dy = py - ay;
        return dx * dx + dy * dy;
    }
    float t = (apx * abx + apy * aby) / abLenSq;
    t = std::max(0.f, std::min(1.f, t));
    float qx = ax + t * abx;
    float qy = ay + t * aby;
    float dx = px - qx;
    float dy = py - qy;
    return dx * dx + dy * dy;
}

void renderLaser(const Laser& laser, IRenderer* renderer, int cameraX, int cameraY) {
    if (!laser.active || laser.pointCount < 2 || renderer == nullptr) {
        return;
    }

    const bool telegraph = laser.age < laser.telegraphDuration;

    int sx[Laser::MAX_POINTS];
    int sy[Laser::MAX_POINTS];
    const int n = laser.pointCount;
    for (int i = 0; i < n; ++i) {
        sx[i] = static_cast<int>(fixedToFloat(laser.points[i].x)) - cameraX;
        sy[i] = static_cast<int>(fixedToFloat(laser.points[i].y)) - cameraY;
    }

#ifndef PLATFORM_PICO
    static const int kOx[4] = {1, -1, 0, 0};
    static const int kOy[4] = {0, 0, 1, -1};
#endif

    for (int i = 0; i < n - 1; ++i) {
        const int x0 = sx[i];
        const int y0 = sy[i];
        const int x1 = sx[i + 1];
        const int y1 = sy[i + 1];
        if (telegraph) {
            renderer->drawLine(x0, y0, x1, y1, Color(90, 20, 20));
        } else {
#ifdef PLATFORM_PICO
            renderer->drawLine(x0 + 1, y0, x1 + 1, y1, Color(120, 0, 0));
            renderer->drawLine(x0, y0, x1, y1, Color(255, 80, 80));
#else
            for (int k = 0; k < 4; ++k) {
                renderer->drawLine(x0 + kOx[k], y0 + kOy[k], x1 + kOx[k], y1 + kOy[k], Color(120, 0, 0));
            }
            renderer->drawLine(x0, y0, x1, y1, Color(255, 80, 80));
            for (int k = 0; k < 4; ++k) {
                renderer->drawLine(x0 + kOx[k], y0 + kOy[k], x1 + kOx[k], y1 + kOy[k], Color(255, 80, 80));
            }
#endif
        }
    }

#ifndef PLATFORM_PICO
    if (!telegraph && n >= 3) {
        for (int i = 1; i < n - 1; ++i) {
            Rect dot(static_cast<float>(sx[i] - 1), static_cast<float>(sy[i] - 1), 3.0f, 3.0f);
            renderer->drawRect(dot, Color(255, 200, 200), true);
        }
    }
#endif
}
