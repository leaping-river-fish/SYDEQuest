#include "DesktopRenderer.h"

DesktopRenderer::DesktopRenderer(SDL_Renderer* renderer, int w, int h)
    : renderer(renderer), screenWidth(w), screenHeight(h)
{
}

void DesktopRenderer::beginFrame() {
    clear(Color(135, 206, 235));  // Sky blue
}

void DesktopRenderer::endFrame() {
    SDL_RenderPresent(renderer);
}

void DesktopRenderer::clear(Color color) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderClear(renderer);
}

void DesktopRenderer::drawRect(const Rect& rect, Color color, bool filled) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    
    SDL_Rect sdlRect;
    sdlRect.x = (int)rect.x;
    sdlRect.y = (int)rect.y;
    sdlRect.w = (int)rect.width;
    sdlRect.h = (int)rect.height;
    
    if (filled) {
        SDL_RenderFillRect(renderer, &sdlRect);
    } else {
        SDL_RenderDrawRect(renderer, &sdlRect);
    }
}

void DesktopRenderer::drawTile(int tileX, int tileY, int tileType,
                                int cameraOffsetX, int cameraOffsetY) {
    if (tileType == 0) return;  // Don't draw air
    
    const int TILE_SIZE = 16;
    Rect rect(
        tileX * TILE_SIZE - cameraOffsetX,
        tileY * TILE_SIZE - cameraOffsetY,
        TILE_SIZE,
        TILE_SIZE
    );
    
    drawRect(rect, getTileColor(tileType), true);
}

Color DesktopRenderer::getTileColor(int tileType) {
    switch (tileType) {
        case 1: return Color(139, 69, 19);   // Brown (solid)
        case 2: return Color(160, 82, 45);   // Lighter brown (platform)
        default: return Color(255, 0, 255);  // Magenta (error)
    }
}

