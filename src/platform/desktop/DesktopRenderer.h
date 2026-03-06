#pragma once
#include "core/IRenderer.h"
#include <SDL2/SDL.h>
#include <vector>

class DesktopRenderer : public IRenderer {
public:
    DesktopRenderer(SDL_Renderer* renderer, int screenWidth, int screenHeight);
    
    void beginFrame() override;
    void endFrame() override;
    void clear(Color color) override;
    void drawRect(const Rect& rect, Color color, bool filled) override;
    void drawTile(int tileX, int tileY, int8_t tileId, int terrainSpritesheet,
                  int cameraOffsetX, int cameraOffsetY) override;
    
    int loadTexture(const char* path) override;
    void drawSprite(int textureID, const Rect& srcRect, const Rect& dstRect, bool flipHorizontal) override;
    
    int getScreenWidth() const override { return screenWidth; }
    int getScreenHeight() const override { return screenHeight; }
    
private:
    SDL_Renderer* renderer;
    int screenWidth;
    int screenHeight;
    std::vector<SDL_Texture*> textures;
};

