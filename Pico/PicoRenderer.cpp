#include "PicoRenderer.h"
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
#include "assets/boss_icon_sprite.h"
#include "assets/title_sprite.h"
#include "assets/boss_sean_sprite.h"
#include "assets/robert_hunter_sprite.h"
#include "assets/game_over_sprite.h"
#include "board_config.h"
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstring>
#include <cctype>

namespace {

/** Basename compare for loadTexture: paths are fake on Pico; only the filename must match. */
bool filenameEqCi(const char* a, const char* b) {
    if (!a || !b) {
        return false;
    }
    for (; *a && *b; ++a, ++b) {
        if (std::tolower(static_cast<unsigned char>(*a)) !=
            std::tolower(static_cast<unsigned char>(*b))) {
            return false;
        }
    }
    return *a == *b;
}

}  // namespace

/** Virtual multi-frame spritesheet: 4 frames per row (matches png_to_rgb565.py sheet export). */
static constexpr int kSpriteSheetCols = 4;

// Array size must match frameCount * frameW * frameH (linear frame layout).
static_assert(sizeof(player_sprite) / sizeof(player_sprite[0]) ==
              static_cast<size_t>(PLAYER_SPRITE_FRAME_COUNT) * PLAYER_SPRITE_FRAME_WIDTH * PLAYER_SPRITE_FRAME_HEIGHT);
static_assert(sizeof(projectile_left_sprite) / sizeof(projectile_left_sprite[0]) ==
              static_cast<size_t>(PROJECTILE_LEFT_SPRITE_FRAME_COUNT) * PROJECTILE_LEFT_SPRITE_FRAME_WIDTH *
                  PROJECTILE_LEFT_SPRITE_FRAME_HEIGHT);
static_assert(sizeof(projectile_right_sprite) / sizeof(projectile_right_sprite[0]) ==
              static_cast<size_t>(PROJECTILE_RIGHT_SPRITE_FRAME_COUNT) * PROJECTILE_RIGHT_SPRITE_FRAME_WIDTH *
                  PROJECTILE_RIGHT_SPRITE_FRAME_HEIGHT);
static_assert(sizeof(terrain_sprite) / sizeof(terrain_sprite[0]) ==
              static_cast<size_t>(TERRAIN_SPRITE_FRAME_COUNT) * TERRAIN_SPRITE_FRAME_WIDTH * TERRAIN_SPRITE_FRAME_HEIGHT);
static_assert(sizeof(enemy_basic_sprite) / sizeof(enemy_basic_sprite[0]) ==
              static_cast<size_t>(ENEMY_BASIC_SPRITE_FRAME_COUNT) * ENEMY_BASIC_SPRITE_FRAME_WIDTH * ENEMY_BASIC_SPRITE_FRAME_HEIGHT);
static_assert(sizeof(enemy_ranged_sprite) / sizeof(enemy_ranged_sprite[0]) ==
              static_cast<size_t>(ENEMY_RANGED_SPRITE_FRAME_COUNT) * ENEMY_RANGED_SPRITE_FRAME_WIDTH * ENEMY_RANGED_SPRITE_FRAME_HEIGHT);
static_assert(sizeof(enemy_projectile_left_sprite) / sizeof(enemy_projectile_left_sprite[0]) ==
              static_cast<size_t>(ENEMY_PROJECTILE_LEFT_SPRITE_FRAME_COUNT) * ENEMY_PROJECTILE_LEFT_SPRITE_FRAME_WIDTH *
                  ENEMY_PROJECTILE_LEFT_SPRITE_FRAME_HEIGHT);
static_assert(sizeof(enemy_projectile_right_sprite) / sizeof(enemy_projectile_right_sprite[0]) ==
              static_cast<size_t>(ENEMY_PROJECTILE_RIGHT_SPRITE_FRAME_COUNT) * ENEMY_PROJECTILE_RIGHT_SPRITE_FRAME_WIDTH *
                  ENEMY_PROJECTILE_RIGHT_SPRITE_FRAME_HEIGHT);
static_assert(sizeof(energy_sprite) / sizeof(energy_sprite[0]) ==
              static_cast<size_t>(ENERGY_SPRITE_FRAME_COUNT) * ENERGY_SPRITE_FRAME_WIDTH * ENERGY_SPRITE_FRAME_HEIGHT);
static_assert(sizeof(health_pack_sprite) / sizeof(health_pack_sprite[0]) ==
              static_cast<size_t>(HEALTH_PACK_SPRITE_FRAME_COUNT) * HEALTH_PACK_SPRITE_FRAME_WIDTH * HEALTH_PACK_SPRITE_FRAME_HEIGHT);
static_assert(sizeof(portal_sprite) / sizeof(portal_sprite[0]) ==
              static_cast<size_t>(PORTAL_SPRITE_FRAME_COUNT) * PORTAL_SPRITE_FRAME_WIDTH * PORTAL_SPRITE_FRAME_HEIGHT);
static_assert(sizeof(objective_charger_sprite) / sizeof(objective_charger_sprite[0]) ==
              static_cast<size_t>(OBJECTIVE_CHARGER_SPRITE_WIDTH) * OBJECTIVE_CHARGER_SPRITE_HEIGHT);
static_assert(sizeof(objective_enclosure_sprite) / sizeof(objective_enclosure_sprite[0]) ==
              static_cast<size_t>(OBJECTIVE_ENCLOSURE_SPRITE_WIDTH) * OBJECTIVE_ENCLOSURE_SPRITE_HEIGHT);
static_assert(sizeof(objective_haptic_sprite) / sizeof(objective_haptic_sprite[0]) ==
              static_cast<size_t>(OBJECTIVE_HAPTIC_SPRITE_WIDTH) * OBJECTIVE_HAPTIC_SPRITE_HEIGHT);
static_assert(sizeof(objective_parts_sprite) / sizeof(objective_parts_sprite[0]) ==
              static_cast<size_t>(OBJECTIVE_PARTS_SPRITE_WIDTH) * OBJECTIVE_PARTS_SPRITE_HEIGHT);
static_assert(sizeof(objective_screen_sprite) / sizeof(objective_screen_sprite[0]) ==
              static_cast<size_t>(OBJECTIVE_SCREEN_SPRITE_WIDTH) * OBJECTIVE_SCREEN_SPRITE_HEIGHT);
static_assert(sizeof(objective_pico_sprite) / sizeof(objective_pico_sprite[0]) ==
              static_cast<size_t>(OBJECTIVE_PICO_SPRITE_WIDTH) * OBJECTIVE_PICO_SPRITE_HEIGHT);
static_assert(sizeof(boss_icon_sprite) / sizeof(boss_icon_sprite[0]) ==
              static_cast<size_t>(BOSS_ICON_SPRITE_WIDTH) * BOSS_ICON_SPRITE_HEIGHT);
static_assert(sizeof(title_sprite) / sizeof(title_sprite[0]) ==
              static_cast<size_t>(TITLE_SPRITE_WIDTH) * TITLE_SPRITE_HEIGHT);
static_assert(sizeof(boss_sean_sprite) / sizeof(boss_sean_sprite[0]) ==
              static_cast<size_t>(BOSS_SEAN_SPRITE_FRAME_COUNT) * BOSS_SEAN_SPRITE_FRAME_WIDTH *
                  BOSS_SEAN_SPRITE_FRAME_HEIGHT);
static_assert(sizeof(robert_hunter_sprite) / sizeof(robert_hunter_sprite[0]) ==
              static_cast<size_t>(ROBERT_HUNTER_SPRITE_FRAME_COUNT) * ROBERT_HUNTER_SPRITE_FRAME_WIDTH *
                  ROBERT_HUNTER_SPRITE_FRAME_HEIGHT);
static_assert(sizeof(game_over_sprite) / sizeof(game_over_sprite[0]) ==
              static_cast<size_t>(GAME_OVER_SPRITE_FRAME_COUNT) * GAME_OVER_SPRITE_FRAME_WIDTH *
                  GAME_OVER_SPRITE_FRAME_HEIGHT);

PicoRenderer::PicoRenderer() : spriteCount(0), bl_pin(13), framebuffer(nullptr) {
    // Physical panel is 240x320 portrait; landscape (320x240) is applied after begin() via setRotation
    // (ST7789_Pico README: do not put 320x240 in Config to fake landscape).
    static constexpr st7789::Rotation kLandscape = st7789::ROTATION_90;

    st7789::Config cfg;
    cfg.spi_inst = spi1;
    // 20 MHz helps rule out signal integrity issues on jumper wiring; increase if stable.
    cfg.spi_speed_hz = 30'000'000;
    cfg.pin_din = 11;
    cfg.pin_sck = 10;
    cfg.pin_cs = 9;
    cfg.pin_dc = 8;
    cfg.pin_reset = 15;
    cfg.pin_bl = 13;
    cfg.width = 240;
    cfg.height = 320;
    cfg.rotation = st7789::ROTATION_0;
    cfg.col_offset = BoardConfig::kSt7789ColOffset;
    cfg.row_offset = BoardConfig::kSt7789RowOffset;
    cfg.ram_col_max = BoardConfig::kSt7789RamColMax;
    cfg.ram_row_max = BoardConfig::kSt7789RamRowMax;
    // DMA off: enabled path caused corruption; HAL uses blocking SPI (same drawImageDMA/fillRectDMA APIs).
    cfg.dma.enabled = false;
    cfg.dma.buffer_size = 4096;
    cfg.rgb_order_bgr = BoardConfig::kSt7789RgbOrderBgr;

    (void)display.begin(cfg);
    display.setRotation(kLandscape);
    assert(display.hal().getConfig().width == 320u && display.hal().getConfig().height == 240u);

    framebuffer = new uint16_t[SCREEN_WIDTH * SCREEN_HEIGHT];
    const uint16_t sky = rgb888_to_rgb565(135, 206, 235);
    for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++) {
        framebuffer[i] = sky;
    }

    pwm_config pwm_cfg = pwm_get_default_config();
    pwm_set_wrap(pwm_gpio_to_slice_num(bl_pin), 65535);
    pwm_init(pwm_gpio_to_slice_num(bl_pin), &pwm_cfg, true);
    gpio_set_function(bl_pin, GPIO_FUNC_PWM);
    pwm_set_gpio_level(bl_pin, 65535);
    
    // IDs match Game::init loadTexture order (0..21)
    registerSprite(0, player_sprite, PLAYER_SPRITE_FRAME_WIDTH, PLAYER_SPRITE_FRAME_HEIGHT, PLAYER_SPRITE_FRAME_COUNT);
    registerSprite(1, projectile_left_sprite, PROJECTILE_LEFT_SPRITE_FRAME_WIDTH, PROJECTILE_LEFT_SPRITE_FRAME_HEIGHT, PROJECTILE_LEFT_SPRITE_FRAME_COUNT);
    registerSprite(2, projectile_right_sprite, PROJECTILE_RIGHT_SPRITE_FRAME_WIDTH, PROJECTILE_RIGHT_SPRITE_FRAME_HEIGHT, PROJECTILE_RIGHT_SPRITE_FRAME_COUNT);
    registerSprite(3, terrain_sprite, TERRAIN_SPRITE_FRAME_WIDTH, TERRAIN_SPRITE_FRAME_HEIGHT, TERRAIN_SPRITE_FRAME_COUNT);
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
    registerSprite(17, boss_icon_sprite, BOSS_ICON_SPRITE_WIDTH, BOSS_ICON_SPRITE_HEIGHT, 1);
    registerSprite(18, title_sprite, TITLE_SPRITE_WIDTH, TITLE_SPRITE_HEIGHT, 1);
    registerSprite(19, boss_sean_sprite, BOSS_SEAN_SPRITE_FRAME_WIDTH, BOSS_SEAN_SPRITE_FRAME_HEIGHT,
                   BOSS_SEAN_SPRITE_FRAME_COUNT);
    registerSprite(20, game_over_sprite, GAME_OVER_SPRITE_FRAME_WIDTH, GAME_OVER_SPRITE_FRAME_HEIGHT,
                   GAME_OVER_SPRITE_FRAME_COUNT);
    registerSprite(21, robert_hunter_sprite, ROBERT_HUNTER_SPRITE_FRAME_WIDTH, ROBERT_HUNTER_SPRITE_FRAME_HEIGHT,
                   ROBERT_HUNTER_SPRITE_FRAME_COUNT);
}

PicoRenderer::~PicoRenderer() {
    delete[] framebuffer;
    framebuffer = nullptr;
}

void PicoRenderer::registerSprite(int id, const uint16_t* data, int width, int height, int frameCount) {
    assert(data != nullptr);
    assert(width > 0 && height > 0 && frameCount > 0);
    if (id >= 0 && id < kMaxSprites) {
        sprites[id].data = data;
        sprites[id].width = width;
        sprites[id].height = height;
        sprites[id].frameCount = frameCount;
        if (id >= spriteCount) spriteCount = id + 1;
    }
}

namespace {

// 8x8 HUD glyphs (scaled 1x or 2x): 0-9, 'x', '/', uppercase subset. MSB = left column.
static const uint8_t HUD_FONT[27][8] = {
    // 0-9
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
    // 'x' multiply / decorative
    {0x00, 0x42, 0x24, 0x18, 0x18, 0x24, 0x42, 0x00},
    {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x00},
    // P, L, A, Y, R, E, T (PLAY / RETRY)
    {0x7C, 0x66, 0x66, 0x7C, 0x60, 0x60, 0x60, 0x00},
    {0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x7E, 0x00},
    {0x18, 0x24, 0x24, 0x3C, 0x24, 0x24, 0x24, 0x00},
    {0x66, 0x66, 0x66, 0x3C, 0x18, 0x18, 0x18, 0x00},
    {0x7C, 0x66, 0x66, 0x7C, 0x6C, 0x66, 0x66, 0x00},
    {0x7E, 0x60, 0x60, 0x7C, 0x60, 0x60, 0x7E, 0x00},
    {0x7E, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00},
    // G, M, O, V (GAME OVER)
    {0x3C, 0x66, 0x60, 0x6E, 0x66, 0x66, 0x3C, 0x00},
    {0x63, 0x77, 0x6B, 0x63, 0x63, 0x63, 0x63, 0x00},
    {0x3C, 0x66, 0x66, 0x66, 0x66, 0x66, 0x3C, 0x00},
    {0x66, 0x66, 0x66, 0x66, 0x66, 0x3C, 0x18, 0x00},
    // S, N, Z, I ("Sean Speziale" / HUD)
    {0x3C, 0x66, 0x60, 0x3C, 0x06, 0x66, 0x3C, 0x00},
    {0x63, 0x73, 0x7B, 0x6F, 0x67, 0x63, 0x63, 0x00},
    {0x7E, 0x06, 0x0C, 0x18, 0x30, 0x60, 0x7E, 0x00},
    {0x3C, 0x18, 0x18, 0x18, 0x18, 0x18, 0x3C, 0x00},
};

static constexpr int HUD_FONT_SCALE = 2;
static constexpr int HUD_FONT_ADVANCE = 8 * HUD_FONT_SCALE;

static int hudGlyphIndex(char c) {
    if (c >= '0' && c <= '9') {
        return c - '0';
    }
    // Multiply symbol: keep 'x' / 'X' before lower->upper so 'x' stays distinct from letters.
    if (c == 'x' || c == 'X') {
        return 10;
    }
    if (c == '/') {
        return 11;
    }
    if (c >= 'a' && c <= 'z') {
        c = static_cast<char>(c - 'a' + 'A');
    }
    switch (c) {
        case 'P': return 12;
        case 'L': return 13;
        case 'A': return 14;
        case 'Y': return 15;
        case 'R': return 16;
        case 'E': return 17;
        case 'T': return 18;
        case 'G': return 19;
        case 'M': return 20;
        case 'O': return 21;
        case 'V': return 22;
        case 'S': return 23;
        case 'N': return 24;
        case 'Z': return 25;
        case 'I': return 26;
        default: return -1;
    }
}

#ifdef PLATFORM_PICO
/** Integer pixel coordinates from fixed (avoids float precision loss vs (int)FROM_FIXED for large values). */
static int fixedToPixelInt(fixed_t v) {
    return static_cast<int>(static_cast<int32_t>(v) >> FIXED_SHIFT);
}
#endif

uint16_t sampleSpritePixel(const SpriteData& sprite, int srcXCoord, int srcYCoord) {
    if (sprite.frameCount <= 1) {
        return sprite.data[srcYCoord * sprite.width + srcXCoord];
    }
    const int fw = sprite.width;
    const int fh = sprite.height;
    int frameCol = srcXCoord / fw;
    int frameRow = srcYCoord / fh;
    int frameIndex = frameRow * kSpriteSheetCols + frameCol;
    if (frameIndex < 0) frameIndex = 0;
    if (frameIndex >= sprite.frameCount) frameIndex = sprite.frameCount - 1;
    int lx = srcXCoord % fw;
    int ly = srcYCoord % fh;
    return sprite.data[frameIndex * fw * fh + ly * fw + lx];
}

} // namespace

void PicoRenderer::blitTransparentPixelsScaled(const uint16_t* base, int sw, int sh, int destX, int destY, int dw,
                                                 int dh, bool flipHorizontal) {
    if (sw <= 0 || sh <= 0 || dw <= 0 || dh <= 0) {
        return;
    }
    const int screenW = logicalWidth();
    const int screenH = logicalHeight();
    constexpr uint16_t kChromaKey = 0xF81F;

    for (int dy = 0; dy < dh; dy++) {
        const int sy = (dy * sh) / dh;
        const int yClamped = sy >= sh ? sh - 1 : sy;
        int screenY = destY + dy;
        if (screenY < 0 || screenY >= screenH) {
            continue;
        }

        for (int dx = 0; dx < dw; dx++) {
            int sx = (dx * sw) / dw;
            if (sx >= sw) {
                sx = sw - 1;
            }
            if (flipHorizontal) {
                sx = sw - 1 - sx;
            }
            uint16_t pixel = base[yClamped * sw + sx];
            if (pixel == kChromaKey) {
                continue;
            }
            int screenX = destX + dx;
            if (screenX >= 0 && screenX < screenW) {
                framebuffer[screenY * SCREEN_WIDTH + screenX] = pixel;
            }
        }
    }
}

void PicoRenderer::blitTransparentPixels(const uint16_t* base, int w, int h, int destX, int destY, bool flipHorizontal) {
    const int sw = logicalWidth();
    const int sh = logicalHeight();

    if (destX >= sw || destY >= sh) return;
    if (destX + w < 0 || destY + h < 0) return;

    int clipLeft = std::max(0, -destX);
    int clipTop = std::max(0, -destY);
    int clipRight = std::min(std::max(0, destX + w - sw), w);
    int clipBottom = std::min(std::max(0, destY + h - sh), h);

    const int drawW = w - clipLeft - clipRight;
    if (drawW <= 0) return;

    for (int dy = clipTop; dy < h - clipBottom; dy++) {
        int screenY = destY + dy;
        if (screenY < 0 || screenY >= sh) continue;

        for (int i = 0; i < drawW; i++) {
            const int dx = clipLeft + i;
            const int screenX = destX + dx;
            if (screenX < 0 || screenX >= sw) continue;

            int lx = flipHorizontal ? (w - 1 - dx) : dx;
            uint16_t pixel = base[dy * w + lx];

            if (pixel != 0xF81F) {
                framebuffer[screenY * SCREEN_WIDTH + screenX] = pixel;
            }
        }
    }
}

void PicoRenderer::clear(Color color) {
    uint16_t rgb565 = rgb888_to_rgb565(color.r, color.g, color.b);
    for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++) {
        framebuffer[i] = rgb565;
    }
}

void PicoRenderer::beginFrame() {
    clear(Color(135, 206, 235));
}

void PicoRenderer::endFrame() {
    // SPI byte order for RGB565 is handled inside HAL (rgb565_to_spi_bytes / rgb565_to_dma_word).
    display.drawImageDMA(0, 0, static_cast<int16_t>(SCREEN_WIDTH), static_cast<int16_t>(SCREEN_HEIGHT), framebuffer);
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
    
#ifdef PLATFORM_PICO
    if (x >= logicalWidth() || y >= logicalHeight()) return;
#else
    if (x >= SCREEN_WIDTH || y >= SCREEN_HEIGHT) return;
#endif
    if (x + w < 0 || y + h < 0) return;
    if (w <= 0 || h <= 0) return;
    
#ifdef PLATFORM_PICO
    const int sw = logicalWidth();
    const int sh = logicalHeight();
#else
    const int sw = SCREEN_WIDTH;
    const int sh = SCREEN_HEIGHT;
#endif

    if (x < 0) { w += x; x = 0; }
    if (y < 0) { h += y; y = 0; }
    if (x + w > sw) w = sw - x;
    if (y + h > sh) h = sh - y;
    
    uint16_t rgb565 = rgb888_to_rgb565(color.r, color.g, color.b);
    
    if (filled) {
        for (int yy = y; yy < y + h; yy++) {
            for (int xx = x; xx < x + w; xx++) {
                framebuffer[yy * SCREEN_WIDTH + xx] = rgb565;
            }
        }
    } else {
        for (int xx = x; xx < x + w; xx++) {
            framebuffer[y * SCREEN_WIDTH + xx] = rgb565;
        }
        if (h > 1) {
            for (int xx = x; xx < x + w; xx++) {
                framebuffer[(y + h - 1) * SCREEN_WIDTH + xx] = rgb565;
            }
        }
        if (h > 2) {
            for (int yy = y + 1; yy < y + h - 1; yy++) {
                framebuffer[yy * SCREEN_WIDTH + x] = rgb565;
                framebuffer[yy * SCREEN_WIDTH + (x + w - 1)] = rgb565;
            }
        }
    }
}

void PicoRenderer::drawLine(int x0, int y0, int x1, int y1, Color color) {
    const int sw = logicalWidth();
    const int sh = logicalHeight();
    uint16_t rgb565 = rgb888_to_rgb565(color.r, color.g, color.b);
    int dx = std::abs(x1 - x0);
    int dy = std::abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx - dy;
    int x = x0;
    int y = y0;
    for (;;) {
        if (x >= 0 && x < sw && y >= 0 && y < sh) {
            framebuffer[y * SCREEN_WIDTH + x] = rgb565;
        }
        if (x == x1 && y == y1) {
            break;
        }
        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x += sx;
        }
        if (e2 < dx) {
            err += dx;
            y += sy;
        }
    }
}

void PicoRenderer::drawSprite(int textureID, const Rect& srcRect, const Rect& dstRect, bool flipHorizontal) {
    if (textureID < 0 || textureID >= spriteCount) return;

    const SpriteData& sprite = sprites[textureID];
    if (!sprite.data) return;

#ifdef PLATFORM_PICO
    int srcX = fixedToPixelInt(srcRect.x);
    int srcY = fixedToPixelInt(srcRect.y);
    int srcW = fixedToPixelInt(srcRect.width);
    int srcH = fixedToPixelInt(srcRect.height);
    int destX = fixedToPixelInt(dstRect.x);
    int destY = fixedToPixelInt(dstRect.y);
#else
    int srcX = static_cast<int>(srcRect.x);
    int srcY = static_cast<int>(srcRect.y);
    int srcW = static_cast<int>(srcRect.width);
    int srcH = static_cast<int>(srcRect.height);
    int destX = static_cast<int>(dstRect.x);
    int destY = static_cast<int>(dstRect.y);
#endif

    const int sheetCols = kSpriteSheetCols;
    const int sheetRows =
        sprite.frameCount > 1 ? (sprite.frameCount + sheetCols - 1) / sheetCols : 1;
    const int sheetW = sprite.frameCount > 1 ? sprite.width * sheetCols : sprite.width;
    const int sheetH = sprite.frameCount > 1 ? sprite.height * sheetRows : sprite.height;

    if (srcX < 0 || srcY < 0 || srcX >= sheetW || srcY >= sheetH) return;
    // Clamp rect to virtual sheet; horizontal loop uses srcW (source rect), never sheetW as span.
    srcW = std::min(srcW, sheetW - srcX);
    srcH = std::min(srcH, sheetH - srcY);
    if (srcW <= 0 || srcH <= 0) return;

    const int sw = logicalWidth();
    const int sh = logicalHeight();

    if (destX >= sw || destY >= sh) return;
    if (destX + srcW < 0 || destY + srcH < 0) return;

    int clipLeft = std::max(0, -destX);
    int clipTop = std::max(0, -destY);
    int clipRight = std::min(std::max(0, destX + srcW - sw), srcW);
    int clipBottom = std::min(std::max(0, destY + srcH - sh), srcH);

    const int drawW = srcW - clipLeft - clipRight;
    if (drawW <= 0) return;

    for (int dy = clipTop; dy < srcH - clipBottom; dy++) {
        int screenY = destY + dy;
        if (screenY < 0 || screenY >= sh) continue;

        for (int i = 0; i < drawW; i++) {
            const int dx = clipLeft + i;
            const int screenX = destX + dx;
            if (screenX < 0 || screenX >= sw) continue;

            int srcXCoord = flipHorizontal ? (srcX + srcW - 1 - dx) : (srcX + dx);
            int srcYCoord = srcY + dy;

            uint16_t pixel = sampleSpritePixel(sprite, srcXCoord, srcYCoord);

            if (pixel != 0xF81F) {
                framebuffer[screenY * SCREEN_WIDTH + screenX] = pixel;
            }
        }
    }
}

void PicoRenderer::drawSpriteFrame(int textureID, int frameIndex, int frameWidth, int frameHeight,
                                   const Rect& dstRect, bool flipHorizontal) {
    (void)frameWidth;
    (void)frameHeight;
    if (textureID < 0 || textureID >= spriteCount) {
        return;
    }
    const SpriteData& sprite = sprites[textureID];
    if (!sprite.data || frameIndex < 0 || frameIndex >= sprite.frameCount) {
        return;
    }
    const int w = sprite.width;
    const int h = sprite.height;
    const uint16_t* base = sprite.data + frameIndex * w * h;

    int destX = fixedToPixelInt(dstRect.x);
    int destY = fixedToPixelInt(dstRect.y);
    int destW = fixedToPixelInt(dstRect.width);
    int destH = fixedToPixelInt(dstRect.height);
    if (destW <= 0) {
        destW = w;
    }
    if (destH <= 0) {
        destH = h;
    }

    if (destW == w && destH == h) {
        blitTransparentPixels(base, w, h, destX, destY, flipHorizontal);
    } else {
        blitTransparentPixelsScaled(base, w, h, destX, destY, destW, destH, flipHorizontal);
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
    if (terrainSpritesheet < 0) return;

    const int TILE_SIZE = 16;
#ifdef PLATFORM_PICO
    // Match drawSpriteFrame: subtract camera in fixed space, then same int conversion as fixedToPixelInt.
    int destX = fixedToPixelInt(TO_FIXED(tileX * TILE_SIZE) - TO_FIXED(cameraOffsetX));
    int destY = fixedToPixelInt(TO_FIXED(tileY * TILE_SIZE) - TO_FIXED(cameraOffsetY));
#else
    int destX = tileX * TILE_SIZE - cameraOffsetX;
    int destY = tileY * TILE_SIZE - cameraOffsetY;
#endif

    if (destX >= logicalWidth() || destY >= logicalHeight()) return;
    if (destX + TILE_SIZE < 0 || destY + TILE_SIZE < 0) return;

    drawFrame(terrainSpritesheet, tileId, destX, destY, false);
}

void PicoRenderer::drawText(const char* text, int x, int y, Color color) {
    drawTextScaled(text, x, y, color, HUD_FONT_SCALE);
}

void PicoRenderer::drawTextScaled(const char* text, int x, int y, Color color, int glyphScalePixels) {
    if (!text || text[0] == '\0') return;
    int scale = glyphScalePixels;
    if (scale < 1) {
        scale = 1;
    }
    if (scale > 2) {
        scale = 2;
    }

    const int sw = logicalWidth();
    const int sh = logicalHeight();
    const uint16_t rgb565 = rgb888_to_rgb565(color.r, color.g, color.b);
    constexpr uint16_t kChromaKey = 0xF81F;
    const int kCell = 8 * scale;
    const int advance = kCell;

    int penX = x;
    for (const char* p = text; *p; ++p) {
        if (*p == ' ') {
            penX += advance;
            continue;
        }
        const int gi = hudGlyphIndex(*p);
        if (gi < 0) {
            continue;
        }

        uint16_t cell[16 * 16];
        for (int i = 0; i < 16 * 16; ++i) {
            cell[i] = kChromaKey;
        }
        const uint8_t* rows = HUD_FONT[gi];
        for (int row = 0; row < 8; ++row) {
            const uint8_t bits = rows[row];
            for (int col = 0; col < 8; ++col) {
                if ((bits & (0x80u >> col)) == 0) continue;
                for (int dy = 0; dy < scale; ++dy) {
                    for (int dx = 0; dx < scale; ++dx) {
                        const int lx = col * scale + dx;
                        const int ly = row * scale + dy;
                        cell[ly * kCell + lx] = rgb565;
                    }
                }
            }
        }
        if (penX < sw && penX + kCell > 0 && y < sh && y + kCell > 0) {
            blitTransparentPixels(cell, kCell, kCell, penX, y, false);
        }
        penX += advance;
    }
}

int PicoRenderer::measureTextWidth(const char* text) const {
    return measureTextWidthScaled(text, HUD_FONT_SCALE);
}

int PicoRenderer::measureTextWidthScaled(const char* text, int glyphScalePixels) const {
    if (!text || !text[0]) {
        return 0;
    }
    int scale = glyphScalePixels;
    if (scale < 1) {
        scale = 1;
    }
    if (scale > 2) {
        scale = 2;
    }
    const int advance = 8 * scale;
    int w = 0;
    for (const char* p = text; *p; ++p) {
        if (*p == ' ') {
            w += advance;
            continue;
        }
        if (hudGlyphIndex(*p) < 0) {
            continue;
        }
        w += advance;
    }
    return w;
}

int PicoRenderer::loadTexture(const char* path) {
    const char* base = path;
    for (const char* p = path; *p; ++p) {
        if (*p == '/' || *p == '\\') {
            base = p + 1;
        }
    }
    // Same order as Game::init(); embedded RGB565 — no disk read, basename must map to an ID.
    if (filenameEqCi(base, "AlternatingWalk.png")) return 0;
    if (filenameEqCi(base, "PencilSpinLeft.png")) return 1;
    if (filenameEqCi(base, "PencilSpinRight.png")) return 2;
    if (filenameEqCi(base, "FullTerrainSpriteSheet.png")) return 3;
    if (filenameEqCi(base, "Circle.png")) return 4;
    if (filenameEqCi(base, "CalcQuiz.png")) return 5;
    if (filenameEqCi(base, "IntegralSpinLeft.png")) return 6;
    if (filenameEqCi(base, "IntegralSpinRight.png")) return 7;
    if (filenameEqCi(base, "Energy.png")) return 8;
    if (filenameEqCi(base, "EnergyDrink.png")) return 9;
    if (filenameEqCi(base, "PortalSpriteSheet.png")) return 10;
    if (filenameEqCi(base, "ChargerSprite.png")) return 11;
    if (filenameEqCi(base, "EnclosureSprite.png")) return 12;
    if (filenameEqCi(base, "HapticSprite.png")) return 13;
    if (filenameEqCi(base, "PartsSprite.png")) return 14;
    if (filenameEqCi(base, "ScreenSprite.png")) return 15;
    if (filenameEqCi(base, "PicoSprite.png")) return 16;
    if (filenameEqCi(base, "Boss.png")) return 17;
    if (filenameEqCi(base, "SYDEQuest.png")) return 18;
    if (filenameEqCi(base, "SeanSpeziale.png")) return 19;
    if (filenameEqCi(base, "GameOver.png")) return 20;
    if (filenameEqCi(base, "Robert Hunter.png")) return 21;
    return -1;
}
