// math.h — core math types.
// vec2/vec3/mat4 etc. will be added as glm aliases in phase 5.
// ivec2 and Color are needed earlier and carry no glm dependency.
#pragma once

#include <cstdint>

namespace glyph {

// Will become `using ivec2 = glm::ivec2` in phase 5.
struct ivec2 {
    int x = 0, y = 0;
};

// Axis-aligned rectangle in float coordinates.
struct Rect { float x, y, w, h; };

// Linear-space RGBA color, components in [0, 1].
struct Color {
    float r = 1.f, g = 1.f, b = 1.f, a = 1.f;

    static Color rgba8(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) {
        return {r / 255.f, g / 255.f, b / 255.f, a / 255.f};
    }
};

} // namespace glyph
