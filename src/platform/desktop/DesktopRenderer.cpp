#include "DesktopRenderer.h"
#include <SDL2/SDL_image.h>

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

void DesktopRenderer::drawTile(int tileX, int tileY, int8_t tileId, int terrainSpritesheet,
                                int cameraOffsetX, int cameraOffsetY) {
    if (tileId == -1) return;  // Don't draw air
    if (terrainSpritesheet < 0) return;  // Invalid texture
    
    const int TILE_SIZE = 16;
    const int TILES_PER_ROW = 4;  // Spritesheet is 4x4 grid
    
    // Calculate source rectangle from spritesheet (2D grid layout)
    int srcX = (tileId % TILES_PER_ROW) * TILE_SIZE;
    int srcY = (tileId / TILES_PER_ROW) * TILE_SIZE;
    
    Rect srcRect(
        srcX,
        srcY,
        TILE_SIZE,
        TILE_SIZE
    );
    
    // Calculate destination rectangle (where to draw on screen)
    Rect dstRect(
        tileX * TILE_SIZE - cameraOffsetX,
        tileY * TILE_SIZE - cameraOffsetY,
        TILE_SIZE,
        TILE_SIZE
    );
    
    // Draw the tile sprite
    drawSprite(terrainSpritesheet, srcRect, dstRect, false);
}

int DesktopRenderer::loadTexture(const char* path) {
    SDL_Surface* surface = IMG_Load(path);
    if (!surface) {
        return -1;
    }
    
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    
    if (!texture) {
        return -1;
    }
    
    textures.push_back(texture);
    return static_cast<int>(textures.size() - 1);
}

void DesktopRenderer::drawSprite(int textureID, const Rect& srcRect, const Rect& dstRect, bool flipHorizontal) {
    if (textureID < 0 || textureID >= static_cast<int>(textures.size())) {
        return;
    }
    
    SDL_Rect src = {
        static_cast<int>(srcRect.x),
        static_cast<int>(srcRect.y),
        static_cast<int>(srcRect.width),
        static_cast<int>(srcRect.height)
    };
    
    SDL_Rect dst = {
        static_cast<int>(dstRect.x),
        static_cast<int>(dstRect.y),
        static_cast<int>(dstRect.width),
        static_cast<int>(dstRect.height)
    };
    
    SDL_RendererFlip flip = flipHorizontal ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
    SDL_RenderCopyEx(renderer, textures[textureID], &src, &dst, 0.0, nullptr, flip);
}
