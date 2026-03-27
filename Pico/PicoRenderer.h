#pragma once
#include "../Shared/core/types.h"
#include "../Shared/core/IRenderer.h"
#include <cstdint>
#include "hardware/spi.h"
#include "hardware/gpio.h"
#include "hardware/dma.h"
#include "hardware/pwm.h"

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
    void drawFrame(int spriteID, int frameIndex, int destX, int destY, bool flipHorizontal = false);
    void drawTile(int tileX, int tileY, int8_t tileId, int terrainSpritesheet,
                  int cameraOffsetX, int cameraOffsetY) override;
    void drawText(const char* text, int x, int y, Color color) override;
    
    int loadTexture(const char* path) override;
    
    int getScreenWidth() const override { return 320; }
    int getScreenHeight() const override { return 240; }
    
private:
    static constexpr int kMaxSprites = 20;
    SpriteData sprites[kMaxSprites];
    int spriteCount;
    
    spi_inst_t* spi;
    uint cs_pin;
    uint dc_pin;
    uint rst_pin;
    uint bl_pin;
    uint dma_channel;
    
    void registerSprite(int id, const uint16_t* data, int width, int height, int frameCount);
    void initDisplay();
    void blitTransparentPixels(const uint16_t* base, int w, int h, int destX, int destY, bool flipHorizontal);
    
    void setWindow(int x, int y, int width, int height);
    void writePixels(const uint16_t* data, size_t count);
    void fillWindow(uint16_t color, size_t count);
    void sendCommand(uint8_t cmd, size_t len = 0, const uint8_t* data = nullptr);
    
    inline uint16_t rgb888_to_rgb565(uint8_t r, uint8_t g, uint8_t b) {
        return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
    }
};
