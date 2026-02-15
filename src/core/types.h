#pragma once
#include <cstdint>

struct Vec2 {
    float x, y;

    Vec2() : x(0), y(0) {}
    Vec2(float x, float y) : x(x), y(y) {}


    Vec2 operator+(const Vec2& other) const {
        return Vec2(x + other.x, y + other.y);
    }

    Vec2 operator*(float scalar) const {
        return Vec2(x * scalar, y * scalar);
    }
};

struct Rect {
    float x, y, width, height;

    Rect() : x(0), y(0), width(0), height(0) {}
    Rect(float x, float y, float width, float height) 
        : x(x), y(y), width(width), height(height) {}

    bool intersects(const Rect& other) const {
        return x < other.x + other.width && 
               x + width > other.x &&
               y < other.y + other.height &&
               y + height > other.y;
    }
};

struct Color {
    uint8_t r, g, b;

    Color(uint8_t r, uint8_t g, uint8_t b) 
        : r(r), g(g), b(b) {}
};