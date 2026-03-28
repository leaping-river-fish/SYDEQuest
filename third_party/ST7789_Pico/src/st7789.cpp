#include "st7789.hpp"
#include <cstdio>

namespace st7789 {

// ST7789 command definitions
enum ST7789_CMD {
    ST7789_NOP     = 0x00,
    ST7789_SWRESET = 0x01,
    ST7789_SLPIN   = 0x10,
    ST7789_SLPOUT  = 0x11,
    ST7789_NORON   = 0x13,
    ST7789_INVOFF  = 0x20,
    ST7789_INVON   = 0x21,
    ST7789_DISPOFF = 0x28,
    ST7789_DISPON  = 0x29,
    ST7789_CASET   = 0x2A,
    ST7789_RASET   = 0x2B,
    ST7789_RAMWR   = 0x2C,
    ST7789_COLMOD  = 0x3A,
    ST7789_MADCTL  = 0x36,
    ST7789_RAMCTRL = 0xB0,
    ST7789_PORCTRL = 0xB2,
    ST7789_GCTRL   = 0xB7,
    ST7789_VCOMS   = 0xBB,
    ST7789_LCMCTRL = 0xC0,
    ST7789_VDVVRHEN= 0xC2,
    ST7789_VRHS    = 0xC3,
    ST7789_VDVS    = 0xC4,
    ST7789_FRCTRL2 = 0xC6,
    ST7789_PWCTRL1 = 0xD0,
    ST7789_PVGAMCTRL = 0xE0,
    ST7789_NVGAMCTRL = 0xE1
};

// MADCTL parameter bit definitions
#define MADCTL_MY  0x80  // Row address order
#define MADCTL_MX  0x40  // Column address order
#define MADCTL_MV  0x20  // Row/Column exchange
#define MADCTL_ML  0x10  // Vertical refresh order
#define MADCTL_RGB 0x00  // RGB order
#define MADCTL_BGR 0x08  // BGR order
#define MADCTL_MH  0x04  // Horizontal refresh order

ST7789::ST7789() : _gfx(this), _initialized(false) {
}

ST7789::~ST7789() {
}

bool ST7789::begin(const Config& config) {
    if (_initialized) {
        return true;
    }
    
    // Initialize hardware abstraction layer
    if (!_hal.init(config)) {
        printf("Failed to initialize hardware abstraction layer\n");
        return false;
    }

    // Must be true before initializeDisplay(): setRotation() and fillRectDMA() early-return when false,
    // so MADCTL/width/height never applied during init and RAM addressing mismatches the game (split/garbage).
    _initialized = true;

    initializeDisplay();
    return true;
}

bool ST7789::begin(spi_inst_t* spi, uint8_t cs_pin, uint8_t dc_pin, 
                  uint8_t rst_pin, uint8_t bl_pin,
                  uint16_t width, uint16_t height) {
    Config config;
    config.spi_inst = spi;
    config.pin_cs = cs_pin;
    config.pin_dc = dc_pin;
    config.pin_reset = rst_pin;
    config.pin_bl = bl_pin;
    config.width = width;
    config.height = height;
    
    return begin(config);
}

void ST7789::initializeDisplay() {
    // Hardware reset
    _hal.reset();
    
    // Software reset
    _hal.writeCommand(ST7789_SWRESET);
    _hal.delay(150);
    
    // Exit sleep mode
    _hal.writeCommand(ST7789_SLPOUT);
    _hal.delay(120);
    
    // Set 16-bit color mode (65K)
    _hal.writeCommand(ST7789_COLMOD);
    _hal.writeData(0x55);  // 16 bits/pixel

    // Orientation + logical width/height (MADCTL). Requires _initialized (see begin()).
    setRotation(_hal.getConfig().rotation);
    
    // Frame rate control
    _hal.writeCommand(ST7789_FRCTRL2);
    _hal.writeData(0x0F);  // 60Hz
    
    // Display inversion
    _hal.writeCommand(ST7789_INVON);
    
    // Other initialization settings
    _hal.writeCommand(ST7789_PORCTRL);
    _hal.writeData(0x0C);
    _hal.writeData(0x0C);
    _hal.writeData(0x00);
    _hal.writeData(0x33);
    _hal.writeData(0x33);
    
    _hal.writeCommand(ST7789_GCTRL);
    _hal.writeData(0x35);
    
    _hal.writeCommand(ST7789_VCOMS);
    _hal.writeData(0x28);
    
    _hal.writeCommand(ST7789_LCMCTRL);
    _hal.writeData(0x0C);
    
    _hal.writeCommand(ST7789_VDVVRHEN);
    _hal.writeData(0x01);
    _hal.writeData(0xFF);
    
    _hal.writeCommand(ST7789_VRHS);
    _hal.writeData(0x10);
    
    _hal.writeCommand(ST7789_VDVS);
    _hal.writeData(0x20);
    
    // Turn on display
    _hal.writeCommand(ST7789_NORON);
    _hal.delay(10);
    
    _hal.writeCommand(ST7789_DISPON);
    _hal.delay(120);
    
    // Set memory window to full screen
    setAddrWindow(0, 0, _hal.getConfig().width - 1, _hal.getConfig().height - 1);
    
    // Clear screen to black
    fillScreen(BLACK);
    
    // Turn on backlight
    setBacklight(true);
}

void ST7789::setAddrWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
    const Config& cfg = _hal.getConfig();
    uint32_t tx0 = static_cast<uint32_t>(x0) + cfg.col_offset;
    uint32_t ty0 = static_cast<uint32_t>(y0) + cfg.row_offset;
    uint32_t tx1 = static_cast<uint32_t>(x1) + cfg.col_offset;
    uint32_t ty1 = static_cast<uint32_t>(y1) + cfg.row_offset;

    if (cfg.ram_col_max > 0u) {
        const uint32_t cmax = static_cast<uint32_t>(cfg.col_offset) + cfg.ram_col_max - 1u;
        if (tx0 > cmax) tx0 = cmax;
        if (tx1 > cmax) tx1 = cmax;
    }
    if (cfg.ram_row_max > 0u) {
        const uint32_t rmax = static_cast<uint32_t>(cfg.row_offset) + cfg.ram_row_max - 1u;
        if (ty0 > rmax) ty0 = rmax;
        if (ty1 > rmax) ty1 = rmax;
    }

    const uint16_t ax0 = static_cast<uint16_t>(tx0 > 0xFFFFu ? 0xFFFFu : tx0);
    const uint16_t ay0 = static_cast<uint16_t>(ty0 > 0xFFFFu ? 0xFFFFu : ty0);
    const uint16_t ax1 = static_cast<uint16_t>(tx1 > 0xFFFFu ? 0xFFFFu : tx1);
    const uint16_t ay1 = static_cast<uint16_t>(ty1 > 0xFFFFu ? 0xFFFFu : ty1);

    // Set column address range
    _hal.writeCommand(ST7789_CASET);
    uint8_t data[4];
    data[0] = (ax0 >> 8) & 0xFF;
    data[1] = ax0 & 0xFF;
    data[2] = (ax1 >> 8) & 0xFF;
    data[3] = ax1 & 0xFF;
    _hal.writeDataBulk(data, 4);
    
    // Set row address range
    _hal.writeCommand(ST7789_RASET);
    data[0] = (ay0 >> 8) & 0xFF;
    data[1] = ay0 & 0xFF;
    data[2] = (ay1 >> 8) & 0xFF;
    data[3] = ay1 & 0xFF;
    _hal.writeDataBulk(data, 4);
    
    // Prepare for memory write
    _hal.writeCommand(ST7789_RAMWR);
}

void ST7789::setRotation(Rotation rotation) {
    if (!_initialized) return;
    
    uint8_t madctl = 0;
    uint16_t old_width = _hal.getConfig().width;
    uint16_t old_height = _hal.getConfig().height;
    
    // Get original width and height in non-rotated state
    uint16_t native_width, native_height;
    if (_hal.getConfig().rotation == ROTATION_0 || _hal.getConfig().rotation == ROTATION_180) {
        // Currently at 0 or 180 degrees
        native_width = old_width;
        native_height = old_height;
    } else {
        // Currently at 90 or 270 degrees
        native_width = old_height;
        native_height = old_width;
    }
    
    // Set MADCTL value for new rotation angle - using values from bak directory
    switch (rotation) {
        case ROTATION_0:
            madctl = 0x00;  // 参考bak目录中的实现
            _hal.setWidth(native_width);
            _hal.setHeight(native_height);
            break;
        case ROTATION_90:
            // 0x60 = landscape (MX|MV), RGB order — common ST7789 / library default.
            madctl = 0x60;
            _hal.setWidth(native_height);
            _hal.setHeight(native_width);
            break;
        case ROTATION_180:
            madctl = 0xC0;  // 参考bak目录中的实现 (0xC0 = 1100 0000)
            _hal.setWidth(native_width);
            _hal.setHeight(native_height);
            break;
        case ROTATION_270:
            madctl = 0xA0;  // alternate landscape (MY|MV), RGB
            _hal.setWidth(native_height);
            _hal.setHeight(native_width);
            break;
    }
    
    _hal.writeCommand(ST7789_MADCTL);
    _hal.writeData(madctl);
    _hal.setRotation(rotation);
    
    // If size has changed, re-set screen window
    if (old_width != _hal.getConfig().width || old_height != _hal.getConfig().height) {
        // Set new address window range to full screen
        setAddrWindow(0, 0, _hal.getConfig().width - 1, _hal.getConfig().height - 1);
    }
}

void ST7789::invertDisplay(bool invert) {
    _hal.writeCommand(invert ? ST7789_INVON : ST7789_INVOFF);
}

void ST7789::fillScreen(uint16_t color) {
    if (_hal.isDmaEnabled()) {
        fillRectDMA(0, 0, _hal.getConfig().width, _hal.getConfig().height, color);
    } else {
        _gfx.fillRect(0, 0, _hal.getConfig().width, _hal.getConfig().height, color);
    }
}

void ST7789::sleepDisplay(bool sleep) {
    _hal.writeCommand(sleep ? ST7789_SLPIN : ST7789_SLPOUT);
    _hal.delay(120);
}

void ST7789::setBacklight(bool on) {
    _hal.setBacklight(on);
}

void ST7789::setBrightness(uint8_t brightness) {
    _hal.setBrightness(brightness);
}

void ST7789::reset() {
    _hal.reset();
    // Re-initialize display
    initializeDisplay();
}

bool ST7789::drawImageDMA(int16_t x, int16_t y, int16_t w, int16_t h, const uint16_t* data) {
    if (!_initialized || x >= _hal.getConfig().width || y >= _hal.getConfig().height) {
        return false;
    }
    
    // Clip coordinates
    int16_t x1 = x + w - 1;
    int16_t y1 = y + h - 1;
    
    if (x < 0) {
        x = 0;
    }
    if (y < 0) {
        y = 0;
    }
    if (x1 >= _hal.getConfig().width) {
        x1 = _hal.getConfig().width - 1;
    }
    if (y1 >= _hal.getConfig().height) {
        y1 = _hal.getConfig().height - 1;
    }
    
    w = x1 - x + 1;
    h = y1 - y + 1;
    
    if (w <= 0 || h <= 0) {
        return false;
    }
    
    // Set drawing window
    setAddrWindow(x, y, x1, y1);
    
    // Use DMA to transfer image data
    return _hal.writeDataDma(data, w * h);
}

bool ST7789::fillRectDMA(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    if (!_initialized || w <= 0 || h <= 0 ||
        x >= _hal.getConfig().width || y >= _hal.getConfig().height) {
        return false;
    }
    
    // Clip coordinates
    int16_t x1 = x + w - 1;
    int16_t y1 = y + h - 1;
    
    if (x < 0) {
        x = 0;
    }
    if (y < 0) {
        y = 0;
    }
    if (x1 >= _hal.getConfig().width) {
        x1 = _hal.getConfig().width - 1;
    }
    if (y1 >= _hal.getConfig().height) {
        y1 = _hal.getConfig().height - 1;
    }
    
    w = x1 - x + 1;
    h = y1 - y + 1;
    
    if (w <= 0 || h <= 0) {
        return false;
    }
    
    // Set drawing window
    setAddrWindow(x, y, x1, y1);

    // One SPI transaction: CS must stay low for the entire RAMWR pixel stream (no repeated RAMWR).
    const size_t total_pixels = static_cast<size_t>(w) * static_cast<size_t>(h);
    return _hal.writeDataDmaSolidColor(color, total_pixels);
}

} // namespace st7789 