#pragma once
#include "core/IRenderer.h"
#include <SDL2/SDL.h>

class DesktopRenderer : public IRenderer {
public:
    DesktopRenderer(SDL_Renderer* renderer, int screenWidth, int screenHeight);
    
    void beginFrame() override;
    void endFrame() override;
    void clear(Color color) override;
    void drawRect(const Rect& rect, Color color, bool filled) override;
    void drawTile(int tileX, int tileY, int tileType,
                  int cameraOffsetX, int cameraOffsetY) override;
    
    int getScreenWidth() const override { return screenWidth; }
    int getScreenHeight() const override { return screenHeight; }
    
private:
    SDL_Renderer* renderer;
    int screenWidth;
    int screenHeight;
    
    Color getTileColor(int tileType);
};

