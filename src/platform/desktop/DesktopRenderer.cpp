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

void DesktopRenderer::drawTile(int tileX, int tileY, int tileType,
                                int cameraOffsetX, int cameraOffsetY) {
    if (tileType == 0) return;  // Don't draw air
    
    const int TILE_SIZE = 16;
    const int PLATFORM_HEIGHT = 8;  // Platforms are half-height
    
    // Platforms are drawn thinner and positioned at the bottom of the tile
    if (tileType == 2) {  // Platform type
        Rect rect(
            tileX * TILE_SIZE - cameraOffsetX,
            tileY * TILE_SIZE + (TILE_SIZE - PLATFORM_HEIGHT) - cameraOffsetY,
            TILE_SIZE,
            PLATFORM_HEIGHT
        );
        drawRect(rect, getTileColor(tileType), true);
    } else {  // Solid tiles
        Rect rect(
            tileX * TILE_SIZE - cameraOffsetX,
            tileY * TILE_SIZE - cameraOffsetY,
            TILE_SIZE,
            TILE_SIZE
        );
        drawRect(rect, getTileColor(tileType), true);
    }
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

Color DesktopRenderer::getTileColor(int tileType) {
    switch (tileType) {
        case 1: return Color(139, 69, 19);   // Brown (solid)
        case 2: return Color(160, 82, 45);   // Lighter brown (platform)
        default: return Color(255, 0, 255);  // Magenta (error)
    }
}

