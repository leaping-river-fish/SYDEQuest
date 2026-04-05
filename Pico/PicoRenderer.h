#pragma once
#include "../Shared/core/types.h"
#include "../Shared/core/IRenderer.h"
#include <cstdint>
#include "st7789.hpp"

struct SpriteData {
    const uint16_t* data;
    int width;
    int height;
    int frameCount;
};

class PicoRenderer : public IRenderer {
public:
    PicoRenderer();
    ~PicoRenderer() override;
    
    void clear(Color color) override;
    void beginFrame() override;
    void endFrame() override;
    
    void drawRect(const Rect& rect, Color color, bool filled) override;
    void drawLine(int x0, int y0, int x1, int y1, Color color) override;
    void drawSprite(int textureID, const Rect& srcRect, const Rect& dstRect, bool flipHorizontal) override;
    void drawSpriteFrame(int textureID, int frameIndex, int frameWidth, int frameHeight,
                         const Rect& dstRect, bool flipHorizontal) override;
    void drawFrame(int spriteID, int frameIndex, int destX, int destY, bool flipHorizontal = false);
    void drawTile(int tileX, int tileY, int8_t tileId, int terrainSpritesheet,
                  int cameraOffsetX, int cameraOffsetY) override;
    void drawText(const char* text, int x, int y, Color color) override;
    int measureTextWidth(const char* text) const override;
    void drawTextScaled(const char* text, int x, int y, Color color, int glyphScalePixels) override;
    int measureTextWidthScaled(const char* text, int glyphScalePixels) const override;

    int loadTexture(const char* path) override;
    
    int getScreenWidth() const override { return SCREEN_WIDTH; }
    int getScreenHeight() const override { return SCREEN_HEIGHT; }
    
private:
    static const int SCREEN_WIDTH = 320;
    static const int SCREEN_HEIGHT = 240;

    /** Logical framebuffer size after ST7789 rotation (landscape); matches CASET/RASET from the driver. */
    int logicalWidth() const { return static_cast<int>(display.hal().getConfig().width); }
    int logicalHeight() const { return static_cast<int>(display.hal().getConfig().height); }

    uint16_t* framebuffer;

    static constexpr int kMaxSprites = 32;
    SpriteData sprites[kMaxSprites];
    int spriteCount;
    
    st7789::ST7789 display;
    uint bl_pin;
    
    void registerSprite(int id, const uint16_t* data, int width, int height, int frameCount);
    void blitTransparentPixels(const uint16_t* base, int w, int h, int destX, int destY, bool flipHorizontal);
    void blitTransparentPixelsScaled(const uint16_t* base, int sw, int sh, int destX, int destY, int dw, int dh,
                                     bool flipHorizontal);
    
    inline uint16_t rgb888_to_rgb565(uint8_t r, uint8_t g, uint8_t b) {
        return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
    }
};
