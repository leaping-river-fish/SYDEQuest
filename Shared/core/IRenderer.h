#pragma once
#include "types.h"
#include <cstdint>

#ifndef PLATFORM_PICO
    #include <string>
#endif

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
    /** 4x3 frame grid: frame index 0..11 maps to (index%4)*fw, (index/4)*fh. */
    virtual void drawSpriteFrame(int textureID, int frameIndex, int frameWidth, int frameHeight,
                                 const Rect& dstRect, bool flipHorizontal) = 0;
    
    virtual void drawText(const char* text, int x, int y, Color color) = 0;
    virtual int measureTextWidth(const char* text) const = 0;

    /** Pico bitmap font: 1 = 8px per glyph, 2 = 16px (default HUD). Desktop ignores scale. */
    virtual void drawTextScaled(const char* text, int x, int y, Color color, int glyphScalePixels) {
        (void)glyphScalePixels;
        drawText(text, x, y, color);
    }
    virtual int measureTextWidthScaled(const char* text, int glyphScalePixels) const {
        (void)glyphScalePixels;
        return measureTextWidth(text);
    }

    virtual int getScreenWidth() const = 0;
    virtual int getScreenHeight() const = 0;
};

