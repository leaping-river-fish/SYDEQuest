#pragma once
#include "Types.h"
#include <string>

class IRenderer {
public:
    virtual ~IRenderer() = default;
    
    virtual void beginFrame() = 0;
    virtual void endFrame() = 0;
    virtual void clear(Color color) = 0;
    
    virtual void drawRect(const Rect& rect, Color color, bool filled) = 0;
    virtual void drawTile(int tileX, int tileY, int8_t tileId, int terrainSpritesheet,
                          int cameraOffsetX, int cameraOffsetY) = 0;
    
    virtual int loadTexture(const char* path) = 0;
    virtual void drawSprite(int textureID, const Rect& srcRect, const Rect& dstRect, bool flipHorizontal) = 0;
    
    virtual void drawText(const std::string& text, int x, int y, Color color) = 0;
    
    virtual int getScreenWidth() const = 0;
    virtual int getScreenHeight() const = 0;
};

