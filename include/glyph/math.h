// math.h — core math types and utilities.
// vec2/vec3/mat4 etc. are aliases over glm so game code can use glm
// operations directly while staying in the glyph namespace.
#pragma once

#include <cstdint>
#include <glm/glm.hpp>

namespace glyph {

using vec2  = glm::vec2;
using vec3  = glm::vec3;
using vec4  = glm::vec4;
using mat3  = glm::mat3;
using mat4  = glm::mat4;
using ivec2 = glm::ivec2;

// Axis-aligned rectangle in float coordinates (y-down, pixel units).
struct Rect {
    float x, y, w, h;

    bool contains(vec2 p) const {
        return p.x >= x && p.x < x + w && p.y >= y && p.y < y + h;
    }
    bool intersects(Rect o) const {
        return x < o.x + o.w && x + w > o.x &&
               y < o.y + o.h && y + h > o.y;
    }
};

// Linear-space RGBA color, components in [0, 1].
struct Color {
    float r = 1.f, g = 1.f, b = 1.f, a = 1.f;

    static Color rgba8(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) {
        return {r / 255.f, g / 255.f, b / 255.f, a / 255.f};
    }
};

inline float lerp(float a, float b, float t) { return a + (b - a) * t; }
inline vec2  lerp(vec2  a, vec2  b, float t) { return a + (b - a) * t; }

} // namespace glyph
