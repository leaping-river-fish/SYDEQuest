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
    void drawSprite(int textureID, const Rect& srcRect, const Rect& dstRect, bool flipHorizontal) override;
    void drawSpriteFrame(int textureID, int frameIndex, int frameWidth, int frameHeight,
                         const Rect& dstRect, bool flipHorizontal) override;
    void drawFrame(int spriteID, int frameIndex, int destX, int destY, bool flipHorizontal = false);
    void drawTile(int tileX, int tileY, int8_t tileId, int terrainSpritesheet,
                  int cameraOffsetX, int cameraOffsetY) override;
    void drawText(const char* text, int x, int y, Color color) override;
    
    int loadTexture(const char* path) override;
    
    int getScreenWidth() const override { return logicalWidth(); }
    int getScreenHeight() const override { return logicalHeight(); }
    
private:
    /** Logical framebuffer size after ST7789 rotation (landscape); matches CASET/RASET from the driver. */
    int logicalWidth() const { return static_cast<int>(display.hal().getConfig().width); }
    int logicalHeight() const { return static_cast<int>(display.hal().getConfig().height); }

    static constexpr int kMaxSprites = 20;
    SpriteData sprites[kMaxSprites];
    int spriteCount;
    
    st7789::ST7789 display;
    uint bl_pin;
    
    void registerSprite(int id, const uint16_t* data, int width, int height, int frameCount);
    void blitTransparentPixels(const uint16_t* base, int w, int h, int destX, int destY, bool flipHorizontal);
    
    inline uint16_t rgb888_to_rgb565(uint8_t r, uint8_t g, uint8_t b) {
        return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
    }
};
