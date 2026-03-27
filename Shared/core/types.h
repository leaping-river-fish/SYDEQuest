#pragma once
#include <cstdint>

#ifdef PLATFORM_PICO
    using fixed_t = int32_t;
    #define FIXED_SHIFT 16
    #define TO_FIXED(x) ((fixed_t)((x) * 65536.0f))
    #define FROM_FIXED(x) ((float)(x) / 65536.0f)
    #define FIXED_MUL(a, b) ((fixed_t)(((int64_t)(a) * (b)) >> FIXED_SHIFT))
    #define FIXED_DIV(a, b) ((fixed_t)(((int64_t)(a) << FIXED_SHIFT) / (b)))
    
    inline fixed_t floatToFixed(float f) { return TO_FIXED(f); }
    inline float fixedToFloat(fixed_t f) { return FROM_FIXED(f); }
#else
    using fixed_t = float;
    #define TO_FIXED(x) (x)
    #define FROM_FIXED(x) (x)
    #define FIXED_MUL(a, b) ((a) * (b))
    #define FIXED_DIV(a, b) ((a) / (b))
    
    inline fixed_t floatToFixed(float f) { return f; }
    inline float fixedToFloat(fixed_t f) { return f; }
#endif

struct Vec2 {
    fixed_t x, y;

    Vec2() : x(0), y(0) {}
    explicit Vec2(fixed_t x, fixed_t y) : x(x), y(y) {}
    #ifdef PLATFORM_PICO
    Vec2(float fx, float fy) : x(floatToFixed(fx)), y(floatToFixed(fy)) {}
    #endif
    Vec2(int ix, int iy) : x(floatToFixed(static_cast<float>(ix))), y(floatToFixed(static_cast<float>(iy))) {}

    Vec2 operator+(const Vec2& other) const {
        Vec2 result;
        result.x = x + other.x;
        result.y = y + other.y;
        return result;
    }
    
    Vec2 operator+=(const Vec2& other) {
        x += other.x;
        y += other.y;
        return *this;
    }
    
    Vec2 operator-(const Vec2& other) const {
        Vec2 result;
        result.x = x - other.x;
        result.y = y - other.y;
        return result;
    }

    Vec2 operator*(fixed_t scalar) const {
        Vec2 result;
        result.x = FIXED_MUL(x, scalar);
        result.y = FIXED_MUL(y, scalar);
        return result;
    }
    
    bool operator!=(fixed_t value) const {
        return x != value || y != value;
    }
};

struct Rect {
    fixed_t x, y, width, height;

    Rect() : x(0), y(0), width(0), height(0) {}
    explicit Rect(fixed_t x, fixed_t y, fixed_t width, fixed_t height) 
        : x(x), y(y), width(width), height(height) {}
    #ifdef PLATFORM_PICO
    Rect(float fx, float fy, float fw, float fh)
        : x(floatToFixed(fx)), y(floatToFixed(fy)), 
          width(floatToFixed(fw)), height(floatToFixed(fh)) {}
    #endif
    Rect(int ix, int iy, int iw, int ih)
        : x(floatToFixed(static_cast<float>(ix))), y(floatToFixed(static_cast<float>(iy))),
          width(floatToFixed(static_cast<float>(iw))), height(floatToFixed(static_cast<float>(ih))) {}

    bool intersects(const Rect& other) const {
        return x < other.x + other.width && 
               x + width > other.x &&
               y < other.y + other.height &&
               y + height > other.y;
    }
};

struct Color {
    uint8_t r, g, b, a;

    Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) 
        : r(r), g(g), b(b), a(a) {}
};