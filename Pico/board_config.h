#pragma once

#include <cstdint>

/**
 * ST7789 panel-specific GRAM addressing (added to CASET/RASET in the driver).
 * Tune for your module datasheet (many 240x320 breakouts use 0,0; some use 52/40).
 */
namespace BoardConfig {

inline constexpr uint16_t kSt7789ColOffset = 0;
inline constexpr uint16_t kSt7789RowOffset = 0;

/** If non-zero, logical column/row addresses after offset are clamped to this GRAM span. */
inline constexpr uint16_t kSt7789RamColMax = 0;
inline constexpr uint16_t kSt7789RamRowMax = 0;

/** ST7789 MADCTL BGR bit; set false if hues are wrong the other way on your panel. */
inline constexpr bool kSt7789RgbOrderBgr = true;

} // namespace BoardConfig
