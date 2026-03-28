#pragma once

#include <cstdint>
#include "hardware/spi.h"
#include "hardware/dma.h"
#include "st7789_config.hpp"

namespace st7789 {

/**
 * SPI stream for ST7789 RAMWR: high byte first, then low byte (matches Graphics paths).
 * DMA path uses rgb565_to_dma_word() so 16-bit TX matches this byte order.
 */
inline void rgb565_to_spi_bytes(uint16_t rgb565, uint8_t out[2]) {
    out[0] = static_cast<uint8_t>(rgb565 >> 8);
    out[1] = static_cast<uint8_t>(rgb565 & 0xFF);
}

/** Word stored in DMA buffer so SPI 16-bit TX emits the same bytes as rgb565_to_spi_bytes. */
inline uint16_t rgb565_to_dma_word(uint16_t rgb565) {
    return static_cast<uint16_t>(((rgb565 & 0xFFu) << 8) | (rgb565 >> 8));
}

// Hardware Abstraction Layer class - handles all hardware-related operations
class HAL {
private:
    Config _config;
    bool _initialized;
    
    // DMA related members
    int _dma_tx_channel;
    uint16_t* _dma_buffer;
    size_t _dma_buffer_size;
    bool _dma_enabled;
    bool _dma_busy;
    
    // Private methods
    void initDma();
    void cleanupDma();
    bool waitForDmaComplete(uint32_t timeout_ms = 1000);
    
public:
    HAL();
    virtual ~HAL();
    
    // Initialize hardware
    bool init(const Config& config);
    
    // Basic IO operations
    void writeCommand(uint8_t cmd);
    void writeData(uint8_t data);
    void writeDataBulk(const uint8_t* data, size_t len);
    
    // DMA operations
    bool writeDataDma(const uint16_t* data, size_t len);
    /** Same SPI transaction semantics as writeDataDma (CS held low for entire transfer) but repeats one RGB565 pixel. */
    bool writeDataDmaSolidColor(uint16_t color, size_t pixel_count);
    bool isDmaBusy() const { return _dma_busy; }
    bool isDmaEnabled() const { return _dma_enabled; }
    void abortDma();
    
    // Hardware control
    void reset();
    void setBacklight(bool on);
    void setBrightness(uint8_t brightness); // If hardware supports PWM
    
    // Spinlock and delay
    void delay(uint32_t ms);
    
    // Get configuration
    const Config& getConfig() const { return _config; }
    
    // Rotation and size settings
    void setWidth(uint16_t width) { _config.width = width; }
    void setHeight(uint16_t height) { _config.height = height; }
    void setRotation(Rotation rotation) { _config.rotation = rotation; }
    
    // Friend declaration - allows interrupt handler to access private members
    friend void dma_complete_handler();
};

} // namespace st7789 