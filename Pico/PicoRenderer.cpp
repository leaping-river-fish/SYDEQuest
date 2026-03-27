#include "PicoRenderer.h"
#include "hardware/spi.h"
#include "hardware/gpio.h"
#include "hardware/dma.h"
#include "hardware/pwm.h"
#include "pico/stdlib.h"
#include "assets/player_sprite.h"
#include "assets/projectile_left_sprite.h"
#include "assets/projectile_right_sprite.h"
#include "assets/terrain_sprite.h"
#include "assets/enemy_basic_sprite.h"
#include "assets/enemy_ranged_sprite.h"
#include "assets/enemy_projectile_left_sprite.h"
#include "assets/enemy_projectile_right_sprite.h"
#include "assets/energy_sprite.h"
#include "assets/health_pack_sprite.h"
#include "assets/portal_sprite.h"
#include "assets/objective_charger_sprite.h"
#include "assets/objective_enclosure_sprite.h"
#include "assets/objective_haptic_sprite.h"
#include "assets/objective_parts_sprite.h"
#include "assets/objective_screen_sprite.h"
#include "assets/objective_pico_sprite.h"
#include <cstring>

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240

#define ST7789_SWRESET 0x01
#define ST7789_SLPOUT 0x11
#define ST7789_INVON 0x21
#define ST7789_DISPON 0x29
#define ST7789_CASET 0x2A
#define ST7789_RASET 0x2B
#define ST7789_RAMWR 0x2C
#define ST7789_MADCTL 0x36
#define ST7789_COLMOD 0x3A
#define ST7789_PORCTRL 0xB2
#define ST7789_GCTRL 0xB7
#define ST7789_VCOMS 0xBB
#define ST7789_LCMCTRL 0xC0
#define ST7789_VDVVRHEN 0xC2
#define ST7789_VRHS 0xC3
#define ST7789_VDVS 0xC4
#define ST7789_FRCTRL2 0xC6
#define ST7789_PWCTRL1 0xD0
#define ST7789_RAMCTRL 0xB0

PicoRenderer::PicoRenderer() : spriteCount(0), spi(spi1), cs_pin(9), dc_pin(8), rst_pin(15), bl_pin(13) {
    spi_init(spi, 62500000);
    
    gpio_set_function(10, GPIO_FUNC_SPI);
    gpio_set_function(11, GPIO_FUNC_SPI);
    
    gpio_init(cs_pin);
    gpio_set_dir(cs_pin, GPIO_OUT);
    gpio_put(cs_pin, 1);
    
    gpio_init(dc_pin);
    gpio_set_dir(dc_pin, GPIO_OUT);
    
    gpio_init(rst_pin);
    gpio_set_dir(rst_pin, GPIO_OUT);
    
    pwm_config cfg = pwm_get_default_config();
    pwm_set_wrap(pwm_gpio_to_slice_num(bl_pin), 65535);
    pwm_init(pwm_gpio_to_slice_num(bl_pin), &cfg, true);
    gpio_set_function(bl_pin, GPIO_FUNC_PWM);
    pwm_set_gpio_level(bl_pin, 0);
    
    dma_channel = dma_claim_unused_channel(true);
    dma_channel_config config = dma_channel_get_default_config(dma_channel);
    channel_config_set_transfer_data_size(&config, DMA_SIZE_8);
    channel_config_set_dreq(&config, spi_get_dreq(spi, true));
    dma_channel_configure(dma_channel, &config, &spi_get_hw(spi)->dr, nullptr, 0, false);
    
    initDisplay();
    
    pwm_set_gpio_level(bl_pin, 65535);
    
    // IDs match Game::init loadTexture order (0..16)
    registerSprite(0, player_sprite, PLAYER_SPRITE_FRAME_WIDTH, PLAYER_SPRITE_FRAME_HEIGHT, PLAYER_SPRITE_FRAME_COUNT);
    registerSprite(1, projectile_left_sprite, PROJECTILE_LEFT_SPRITE_FRAME_WIDTH, PROJECTILE_LEFT_SPRITE_FRAME_HEIGHT, PROJECTILE_LEFT_SPRITE_FRAME_COUNT);
    registerSprite(2, projectile_right_sprite, PROJECTILE_RIGHT_SPRITE_FRAME_WIDTH, PROJECTILE_RIGHT_SPRITE_FRAME_HEIGHT, PROJECTILE_RIGHT_SPRITE_FRAME_COUNT);
    registerSprite(3, terrain_sprite, TERRAIN_SPRITE_WIDTH, TERRAIN_SPRITE_HEIGHT, 1);
    registerSprite(4, enemy_basic_sprite, ENEMY_BASIC_SPRITE_FRAME_WIDTH, ENEMY_BASIC_SPRITE_FRAME_HEIGHT, ENEMY_BASIC_SPRITE_FRAME_COUNT);
    registerSprite(5, enemy_ranged_sprite, ENEMY_RANGED_SPRITE_FRAME_WIDTH, ENEMY_RANGED_SPRITE_FRAME_HEIGHT, ENEMY_RANGED_SPRITE_FRAME_COUNT);
    registerSprite(6, enemy_projectile_left_sprite, ENEMY_PROJECTILE_LEFT_SPRITE_FRAME_WIDTH, ENEMY_PROJECTILE_LEFT_SPRITE_FRAME_HEIGHT, ENEMY_PROJECTILE_LEFT_SPRITE_FRAME_COUNT);
    registerSprite(7, enemy_projectile_right_sprite, ENEMY_PROJECTILE_RIGHT_SPRITE_FRAME_WIDTH, ENEMY_PROJECTILE_RIGHT_SPRITE_FRAME_HEIGHT, ENEMY_PROJECTILE_RIGHT_SPRITE_FRAME_COUNT);
    registerSprite(8, energy_sprite, ENERGY_SPRITE_FRAME_WIDTH, ENERGY_SPRITE_FRAME_HEIGHT, ENERGY_SPRITE_FRAME_COUNT);
    registerSprite(9, health_pack_sprite, HEALTH_PACK_SPRITE_FRAME_WIDTH, HEALTH_PACK_SPRITE_FRAME_HEIGHT, HEALTH_PACK_SPRITE_FRAME_COUNT);
    registerSprite(10, portal_sprite, PORTAL_SPRITE_FRAME_WIDTH, PORTAL_SPRITE_FRAME_HEIGHT, PORTAL_SPRITE_FRAME_COUNT);
    registerSprite(11, objective_charger_sprite, OBJECTIVE_CHARGER_SPRITE_WIDTH, OBJECTIVE_CHARGER_SPRITE_HEIGHT, 1);
    registerSprite(12, objective_enclosure_sprite, OBJECTIVE_ENCLOSURE_SPRITE_WIDTH, OBJECTIVE_ENCLOSURE_SPRITE_HEIGHT, 1);
    registerSprite(13, objective_haptic_sprite, OBJECTIVE_HAPTIC_SPRITE_WIDTH, OBJECTIVE_HAPTIC_SPRITE_HEIGHT, 1);
    registerSprite(14, objective_parts_sprite, OBJECTIVE_PARTS_SPRITE_WIDTH, OBJECTIVE_PARTS_SPRITE_HEIGHT, 1);
    registerSprite(15, objective_screen_sprite, OBJECTIVE_SCREEN_SPRITE_WIDTH, OBJECTIVE_SCREEN_SPRITE_HEIGHT, 1);
    registerSprite(16, objective_pico_sprite, OBJECTIVE_PICO_SPRITE_WIDTH, OBJECTIVE_PICO_SPRITE_HEIGHT, 1);
}

PicoRenderer::~PicoRenderer() {
    if (dma_channel_is_claimed(dma_channel)) {
        dma_channel_abort(dma_channel);
        dma_channel_unclaim(dma_channel);
    }
}

void PicoRenderer::registerSprite(int id, const uint16_t* data, int width, int height, int frameCount) {
    if (id >= 0 && id < kMaxSprites) {
        sprites[id].data = data;
        sprites[id].width = width;
        sprites[id].height = height;
        sprites[id].frameCount = frameCount;
        if (id >= spriteCount) spriteCount = id + 1;
    }
}

namespace {

// 8x8 HUD glyphs: 0-9, then 'x' (for "3x" style HP text). MSB = left column.
static const uint8_t HUD_FONT[11][8] = {
    {0x3C, 0x66, 0x66, 0x66, 0x66, 0x66, 0x3C, 0x00},
    {0x18, 0x38, 0x18, 0x18, 0x18, 0x18, 0x7E, 0x00},
    {0x3C, 0x66, 0x06, 0x0C, 0x18, 0x30, 0x7E, 0x00},
    {0x3C, 0x66, 0x06, 0x1C, 0x06, 0x66, 0x3C, 0x00},
    {0x0C, 0x1C, 0x2C, 0x4C, 0x7E, 0x0C, 0x0C, 0x00},
    {0x7E, 0x60, 0x7C, 0x06, 0x06, 0x66, 0x3C, 0x00},
    {0x3C, 0x60, 0x60, 0x7C, 0x66, 0x66, 0x3C, 0x00},
    {0x7E, 0x06, 0x0C, 0x18, 0x30, 0x30, 0x30, 0x00},
    {0x3C, 0x66, 0x66, 0x3C, 0x66, 0x66, 0x3C, 0x00},
    {0x3C, 0x66, 0x66, 0x3E, 0x06, 0x06, 0x3C, 0x00},
    {0x00, 0x42, 0x24, 0x18, 0x18, 0x24, 0x42, 0x00},
};

static constexpr int HUD_FONT_ADVANCE = 8;

static int hudGlyphIndex(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c == 'x' || c == 'X') return 10;
    return -1;
}

uint16_t sampleSpritePixel(const SpriteData& sprite, int srcXCoord, int srcYCoord) {
    if (sprite.frameCount <= 1) {
        return sprite.data[srcYCoord * sprite.width + srcXCoord];
    }
    const int fw = sprite.width;
    const int fh = sprite.height;
    int frameCol = srcXCoord / fw;
    int frameRow = srcYCoord / fh;
    int frameIndex = frameRow * 4 + frameCol;
    if (frameIndex < 0) frameIndex = 0;
    if (frameIndex >= sprite.frameCount) frameIndex = sprite.frameCount - 1;
    int lx = srcXCoord % fw;
    int ly = srcYCoord % fh;
    return sprite.data[frameIndex * fw * fh + ly * fw + lx];
}

} // namespace

void PicoRenderer::blitTransparentPixels(const uint16_t* base, int w, int h, int destX, int destY, bool flipHorizontal) {
    if (destX >= SCREEN_WIDTH || destY >= SCREEN_HEIGHT) return;
    if (destX + w < 0 || destY + h < 0) return;

    int clipLeft = destX < 0 ? -destX : 0;
    int clipTop = destY < 0 ? -destY : 0;
    int clipRight = (destX + w > SCREEN_WIDTH) ? (destX + w - SCREEN_WIDTH) : 0;
    int clipBottom = (destY + h > SCREEN_HEIGHT) ? (destY + h - SCREEN_HEIGHT) : 0;

    static uint16_t lineBuffer[320];

    for (int dy = clipTop; dy < h - clipBottom; dy++) {
        int screenY = destY + dy;
        if (screenY < 0 || screenY >= SCREEN_HEIGHT) continue;

        int bufferCount = 0;
        int lineStartX = -1;

        for (int dx = clipLeft; dx < w - clipRight; dx++) {
            int screenX = destX + dx;
            if (screenX < 0 || screenX >= SCREEN_WIDTH) continue;

            int lx = flipHorizontal ? (w - 1 - dx) : dx;
            uint16_t pixel = base[dy * w + lx];

            if (pixel != 0xF81F) {
                if (lineStartX == -1) {
                    lineStartX = screenX;
                }
                lineBuffer[bufferCount++] = __builtin_bswap16(pixel);
            } else {
                if (bufferCount > 0 && lineStartX >= 0 && lineStartX < SCREEN_WIDTH) {
                    int actualWidth = bufferCount;
                    if (lineStartX + actualWidth > SCREEN_WIDTH) {
                        actualWidth = SCREEN_WIDTH - lineStartX;
                    }
                    if (actualWidth > 0) {
                        setWindow(lineStartX, screenY, actualWidth, 1);
                        writePixels(lineBuffer, actualWidth);
                    }
                    bufferCount = 0;
                    lineStartX = -1;
                }
            }
        }

        if (bufferCount > 0 && lineStartX >= 0 && lineStartX < SCREEN_WIDTH) {
            int actualWidth = bufferCount;
            if (lineStartX + actualWidth > SCREEN_WIDTH) {
                actualWidth = SCREEN_WIDTH - lineStartX;
            }
            if (actualWidth > 0) {
                setWindow(lineStartX, screenY, actualWidth, 1);
                writePixels(lineBuffer, actualWidth);
            }
        }
    }
}

void PicoRenderer::sendCommand(uint8_t cmd, size_t len, const uint8_t* data) {
    gpio_put(dc_pin, 0);
    gpio_put(cs_pin, 0);
    
    spi_write_blocking(spi, &cmd, 1);
    
    if (data && len > 0) {
        gpio_put(dc_pin, 1);
        spi_write_blocking(spi, data, len);
    }
    
    gpio_put(cs_pin, 1);
}

void PicoRenderer::initDisplay() {
    gpio_put(rst_pin, 1);
    sleep_ms(5);
    gpio_put(rst_pin, 0);
    sleep_ms(20);
    gpio_put(rst_pin, 1);
    sleep_ms(150);
    
    sendCommand(ST7789_SWRESET);
    sleep_ms(150);
    
    sendCommand(0x35);
    
    uint8_t colmod = 0x55;
    sendCommand(ST7789_COLMOD, 1, &colmod);
    
    uint8_t porctrl[5] = {0x0c, 0x0c, 0x00, 0x33, 0x33};
    sendCommand(ST7789_PORCTRL, 5, porctrl);
    
    uint8_t gctrl = 0x35;
    sendCommand(ST7789_GCTRL, 1, &gctrl);
    
    uint8_t vcoms = 0x1f;
    sendCommand(ST7789_VCOMS, 1, &vcoms);
    
    uint8_t lcmctrl = 0x2c;
    sendCommand(ST7789_LCMCTRL, 1, &lcmctrl);
    
    uint8_t vdvvrhen = 0x01;
    sendCommand(ST7789_VDVVRHEN, 1, &vdvvrhen);
    
    uint8_t vrhs = 0x12;
    sendCommand(ST7789_VRHS, 1, &vrhs);
    
    uint8_t vdvs = 0x20;
    sendCommand(ST7789_VDVS, 1, &vdvs);
    
    uint8_t pwctrl1[2] = {0xa4, 0xa1};
    sendCommand(ST7789_PWCTRL1, 2, pwctrl1);
    
    uint8_t frctrl2 = 0x0f;
    sendCommand(ST7789_FRCTRL2, 1, &frctrl2);
    
    uint8_t ramctrl[2] = {0x00, 0xc0};
    sendCommand(ST7789_RAMCTRL, 2, ramctrl);
    
    uint8_t gmctrp1[14] = {0xD0, 0x08, 0x11, 0x08, 0x0C, 0x15, 0x39, 0x33, 0x50, 0x36, 0x13, 0x14, 0x29, 0x2D};
    sendCommand(0xE0, 14, gmctrp1);
    
    uint8_t gmctrn1[14] = {0xD0, 0x08, 0x10, 0x08, 0x06, 0x06, 0x39, 0x44, 0x51, 0x0B, 0x16, 0x14, 0x2F, 0x31};
    sendCommand(0xE1, 14, gmctrn1);
    
    uint8_t madctl = 0x70;
    sendCommand(ST7789_MADCTL, 1, &madctl);
    
    uint8_t caset[4] = {0x00, 0x00, 0x01, 0x3F};
    sendCommand(ST7789_CASET, 4, caset);
    
    uint8_t raset[4] = {0x00, 0x00, 0x00, 0xEF};
    sendCommand(ST7789_RASET, 4, raset);
    
    sendCommand(ST7789_INVON);
    sendCommand(ST7789_SLPOUT);
    sleep_ms(120);
    
    sendCommand(ST7789_DISPON);
    sleep_ms(100);
    
    setWindow(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    uint16_t skyBlue = rgb888_to_rgb565(135, 206, 235);
    fillWindow(skyBlue, SCREEN_WIDTH * SCREEN_HEIGHT);
    sleep_ms(10);
}

void PicoRenderer::setWindow(int x, int y, int width, int height) {
    if (width <= 0 || height <= 0) return;
    
    int x1 = (x < 0) ? 0 : x;
    int y1 = (y < 0) ? 0 : y;
    int x2 = x + width - 1;
    int y2 = y + height - 1;
    
    if (x1 >= SCREEN_WIDTH || y1 >= SCREEN_HEIGHT) return;
    if (x2 < 0 || y2 < 0) return;
    
    if (x2 >= SCREEN_WIDTH) x2 = SCREEN_WIDTH - 1;
    if (y2 >= SCREEN_HEIGHT) y2 = SCREEN_HEIGHT - 1;
    if (x2 < x1 || y2 < y1) return;
    
    uint8_t caset_data[4] = {
        (uint8_t)(x1 >> 8),
        (uint8_t)(x1 & 0xFF),
        (uint8_t)(x2 >> 8),
        (uint8_t)(x2 & 0xFF)
    };
    
    uint8_t raset_data[4] = {
        (uint8_t)(y1 >> 8),
        (uint8_t)(y1 & 0xFF),
        (uint8_t)(y2 >> 8),
        (uint8_t)(y2 & 0xFF)
    };
    
    sendCommand(ST7789_CASET, 4, caset_data);
    sendCommand(ST7789_RASET, 4, raset_data);
}

void PicoRenderer::writePixels(const uint16_t* data, size_t count) {
    if (count == 0) return;
    
    gpio_put(dc_pin, 0);
    gpio_put(cs_pin, 0);
    
    uint8_t cmd = ST7789_RAMWR;
    spi_write_blocking(spi, &cmd, 1);
    
    gpio_put(dc_pin, 1);
    
    size_t byte_count = count * 2;
    while (dma_channel_is_busy(dma_channel));
    
    dma_channel_set_trans_count(dma_channel, byte_count, false);
    dma_channel_set_read_addr(dma_channel, data, true);
    dma_channel_wait_for_finish_blocking(dma_channel);
    
    gpio_put(cs_pin, 1);
}

void PicoRenderer::fillWindow(uint16_t color, size_t count) {
    if (count == 0) return;
    
    gpio_put(dc_pin, 0);
    gpio_put(cs_pin, 0);
    
    uint8_t cmd = ST7789_RAMWR;
    spi_write_blocking(spi, &cmd, 1);
    
    gpio_put(dc_pin, 1);
    
    uint16_t swapped = __builtin_bswap16(color);
    
    static uint16_t fillBuffer[320];
    for (int i = 0; i < 320; i++) {
        fillBuffer[i] = swapped;
    }
    
    size_t remaining = count;
    while (remaining > 0) {
        size_t chunk = (remaining > 320) ? 320 : remaining;
        
        while (dma_channel_is_busy(dma_channel));
        dma_channel_set_trans_count(dma_channel, chunk * 2, false);
        dma_channel_set_read_addr(dma_channel, fillBuffer, true);
        dma_channel_wait_for_finish_blocking(dma_channel);
        
        remaining -= chunk;
    }
    
    gpio_put(cs_pin, 1);
}

void PicoRenderer::clear(Color color) {
    setWindow(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    uint16_t rgb565 = rgb888_to_rgb565(color.r, color.g, color.b);
    fillWindow(rgb565, SCREEN_WIDTH * SCREEN_HEIGHT);
}

void PicoRenderer::beginFrame() {
}

void PicoRenderer::endFrame() {
}

void PicoRenderer::drawRect(const Rect& rect, Color color, bool filled) {
#ifdef PLATFORM_PICO
    int x = FROM_FIXED(rect.x);
    int y = FROM_FIXED(rect.y);
    int w = FROM_FIXED(rect.width);
    int h = FROM_FIXED(rect.height);
#else
    int x = static_cast<int>(rect.x);
    int y = static_cast<int>(rect.y);
    int w = static_cast<int>(rect.width);
    int h = static_cast<int>(rect.height);
#endif
    
    if (x >= SCREEN_WIDTH || y >= SCREEN_HEIGHT) return;
    if (x + w < 0 || y + h < 0) return;
    if (w <= 0 || h <= 0) return;
    
    if (x < 0) { w += x; x = 0; }
    if (y < 0) { h += y; y = 0; }
    if (x + w > SCREEN_WIDTH) w = SCREEN_WIDTH - x;
    if (y + h > SCREEN_HEIGHT) h = SCREEN_HEIGHT - y;
    
    uint16_t rgb565 = rgb888_to_rgb565(color.r, color.g, color.b);
    
    if (filled) {
        setWindow(x, y, w, h);
        fillWindow(rgb565, w * h);
    } else {
        setWindow(x, y, w, 1);
        fillWindow(rgb565, w);
        
        if (h > 1) {
            setWindow(x, y + h - 1, w, 1);
            fillWindow(rgb565, w);
        }
        
        if (h > 2) {
            setWindow(x, y + 1, 1, h - 2);
            fillWindow(rgb565, h - 2);
            
            setWindow(x + w - 1, y + 1, 1, h - 2);
            fillWindow(rgb565, h - 2);
        }
    }
}

void PicoRenderer::drawSprite(int textureID, const Rect& srcRect, const Rect& dstRect, bool flipHorizontal) {
    if (textureID < 0 || textureID >= spriteCount) return;
    
    const SpriteData& sprite = sprites[textureID];
    if (!sprite.data) return;
    
#ifdef PLATFORM_PICO
    int srcX = FROM_FIXED(srcRect.x);
    int srcY = FROM_FIXED(srcRect.y);
    int srcW = FROM_FIXED(srcRect.width);
    int srcH = FROM_FIXED(srcRect.height);
    int destX = FROM_FIXED(dstRect.x);
    int destY = FROM_FIXED(dstRect.y);
#else
    int srcX = static_cast<int>(srcRect.x);
    int srcY = static_cast<int>(srcRect.y);
    int srcW = static_cast<int>(srcRect.width);
    int srcH = static_cast<int>(srcRect.height);
    int destX = static_cast<int>(dstRect.x);
    int destY = static_cast<int>(dstRect.y);
#endif
    
    int sheetW = sprite.frameCount > 1 ? sprite.width * 4 : sprite.width;
    int sheetH = sprite.frameCount > 1 ? sprite.height * 3 : sprite.height;
    if (srcX < 0 || srcY < 0 || srcX + srcW > sheetW || srcY + srcH > sheetH) return;
    if (destX >= SCREEN_WIDTH || destY >= SCREEN_HEIGHT) return;
    if (destX + srcW < 0 || destY + srcH < 0) return;
    
    int clipLeft = destX < 0 ? -destX : 0;
    int clipTop = destY < 0 ? -destY : 0;
    int clipRight = (destX + srcW > SCREEN_WIDTH) ? (destX + srcW - SCREEN_WIDTH) : 0;
    int clipBottom = (destY + srcH > SCREEN_HEIGHT) ? (destY + srcH - SCREEN_HEIGHT) : 0;
    
    static uint16_t lineBuffer[320];
    
    for (int dy = clipTop; dy < srcH - clipBottom; dy++) {
        int screenY = destY + dy;
        if (screenY < 0 || screenY >= SCREEN_HEIGHT) continue;
        
        int bufferCount = 0;
        int lineStartX = -1;
        
        for (int dx = clipLeft; dx < srcW - clipRight; dx++) {
            int screenX = destX + dx;
            if (screenX < 0 || screenX >= SCREEN_WIDTH) continue;
            
            int srcXCoord = flipHorizontal ? (srcX + srcW - 1 - dx) : (srcX + dx);
            int srcYCoord = srcY + dy;
            
            uint16_t pixel = sampleSpritePixel(sprite, srcXCoord, srcYCoord);
            
            if (pixel != 0xF81F) {
                if (lineStartX == -1) {
                    lineStartX = screenX;
                }
                lineBuffer[bufferCount++] = __builtin_bswap16(pixel);
            } else {
                if (bufferCount > 0 && lineStartX >= 0 && lineStartX < SCREEN_WIDTH) {
                    int actualWidth = bufferCount;
                    if (lineStartX + actualWidth > SCREEN_WIDTH) {
                        actualWidth = SCREEN_WIDTH - lineStartX;
                    }
                    if (actualWidth > 0) {
                        setWindow(lineStartX, screenY, actualWidth, 1);
                        writePixels(lineBuffer, actualWidth);
                    }
                    bufferCount = 0;
                    lineStartX = -1;
                }
            }
        }
        
        if (bufferCount > 0 && lineStartX >= 0 && lineStartX < SCREEN_WIDTH) {
            int actualWidth = bufferCount;
            if (lineStartX + actualWidth > SCREEN_WIDTH) {
                actualWidth = SCREEN_WIDTH - lineStartX;
            }
            if (actualWidth > 0) {
                setWindow(lineStartX, screenY, actualWidth, 1);
                writePixels(lineBuffer, actualWidth);
            }
        }
    }
}

void PicoRenderer::drawFrame(int spriteID, int frameIndex, int destX, int destY, bool flipHorizontal) {
    if (spriteID < 0 || spriteID >= spriteCount) return;

    const SpriteData& sprite = sprites[spriteID];
    if (!sprite.data || frameIndex < 0 || frameIndex >= sprite.frameCount) return;

    const int w = sprite.width;
    const int h = sprite.height;
    const uint16_t* base = sprite.data + frameIndex * w * h;
    blitTransparentPixels(base, w, h, destX, destY, flipHorizontal);
}

void PicoRenderer::drawTile(int tileX, int tileY, int8_t tileId, int terrainSpritesheet,
                            int cameraOffsetX, int cameraOffsetY) {
    if (tileId < 0 || tileId > 15) return;

    // Must match DesktopRenderer::drawTile: 4x4 grid of 16x16 tiles on 64x64 sheet
    const int TILES_PER_ROW = 4;
    const int TILE_SIZE = 16;
    int srcX = (tileId % TILES_PER_ROW) * TILE_SIZE;
    int srcY = (tileId / TILES_PER_ROW) * TILE_SIZE;
    
    int destX = tileX * TILE_SIZE - cameraOffsetX;
    int destY = tileY * TILE_SIZE - cameraOffsetY;
    
    if (destX >= SCREEN_WIDTH || destY >= SCREEN_HEIGHT) return;
    if (destX + TILE_SIZE < 0 || destY + TILE_SIZE < 0) return;
    
    Rect srcRect(static_cast<float>(srcX), static_cast<float>(srcY), static_cast<float>(TILE_SIZE), static_cast<float>(TILE_SIZE));
    Rect dstRect(static_cast<float>(destX), static_cast<float>(destY), static_cast<float>(TILE_SIZE), static_cast<float>(TILE_SIZE));
    
    drawSprite(terrainSpritesheet, srcRect, dstRect, false);
}

void PicoRenderer::drawText(const char* text, int x, int y, Color color) {
    if (!text || text[0] == '\0') return;

    uint16_t rgb565 = rgb888_to_rgb565(color.r, color.g, color.b);
    uint16_t swapped = __builtin_bswap16(rgb565);

    int penX = x;
    for (const char* p = text; *p; ++p) {
        if (*p == ' ') {
            penX += HUD_FONT_ADVANCE;
            continue;
        }
        int gi = hudGlyphIndex(*p);
        if (gi < 0) {
            continue;
        }

        const uint8_t* rows = HUD_FONT[gi];
        for (int row = 0; row < 8; ++row) {
            uint8_t bits = rows[row];
            for (int col = 0; col < 8; ++col) {
                if ((bits & (0x80u >> col)) == 0) continue;
                int sx = penX + col;
                int sy = y + row;
                if (sx < 0 || sx >= SCREEN_WIDTH || sy < 0 || sy >= SCREEN_HEIGHT) continue;
                setWindow(sx, sy, 1, 1);
                writePixels(&swapped, 1);
            }
        }
        penX += HUD_FONT_ADVANCE;
    }
}

int PicoRenderer::loadTexture(const char* path) {
    const char* base = path;
    for (const char* p = path; *p; ++p) {
        if (*p == '/' || *p == '\\') {
            base = p + 1;
        }
    }
    // Same order as Game::init()
    if (strcmp(base, "AlternatingWalk.png") == 0) return 0;
    if (strcmp(base, "PencilSpinLeft.png") == 0) return 1;
    if (strcmp(base, "PencilSpinRight.png") == 0) return 2;
    if (strcmp(base, "FullTerrainSpriteSheet.png") == 0) return 3;
    if (strcmp(base, "Circle.png") == 0) return 4;
    if (strcmp(base, "CalcQuiz.png") == 0) return 5;
    if (strcmp(base, "IntegralSpinLeft.png") == 0) return 6;
    if (strcmp(base, "IntegralSpinRight.png") == 0) return 7;
    if (strcmp(base, "Energy.png") == 0) return 8;
    if (strcmp(base, "EnergyDrink.png") == 0) return 9;
    if (strcmp(base, "PortalSpriteSheet.png") == 0) return 10;
    if (strcmp(base, "ChargerSprite.png") == 0) return 11;
    if (strcmp(base, "EnclosureSprite.png") == 0) return 12;
    if (strcmp(base, "HapticSprite.png") == 0) return 13;
    if (strcmp(base, "PartsSprite.png") == 0) return 14;
    if (strcmp(base, "ScreenSprite.png") == 0) return 15;
    if (strcmp(base, "PicoSprite.png") == 0) return 16;
    return -1;
}
