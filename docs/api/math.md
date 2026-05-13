# Math Types

**Header:** `#include <glyph/math.h>`

Lightweight types for 2D coordinates and colors. In phase 5, `vec2`/`vec3`/`mat4` will be added as `glm` aliases.

---

## Color

Linear-space RGBA color. Components are `float` in `[0.0, 1.0]`.

```cpp
struct Color {
    float r = 1.f, g = 1.f, b = 1.f, a = 1.f;

    static Color rgba8(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255);
};
```

### `Color::rgba8(r, g, b, a)`

Convert 8-bit RGBA values (0–255) to a `Color`. The most convenient way to specify colors.

```cpp
glyph::Color red   = glyph::Color::rgba8(255,   0,   0);
glyph::Color white = glyph::Color::rgba8(255, 255, 255);
glyph::Color semi  = glyph::Color::rgba8(255, 255, 255, 128); // 50% transparent
```

### Direct construction

```cpp
glyph::Color c;          // white, fully opaque (default)
glyph::Color black{0, 0, 0, 1};
```

---

## Rect

Axis-aligned rectangle in float coordinates.

```cpp
struct Rect { float x, y, w, h; };
```

| Field | Description |
|---|---|
| `x`, `y` | Top-left corner in pixel/world space |
| `w`, `h` | Width and height |

```cpp
glyph::Rect screen_quad{0.f, 0.f, 1280.f, 720.f};   // full screen
glyph::Rect sprite_dest{100.f, 200.f, 64.f, 64.f};   // 64×64 at (100, 200)
```

---

## ivec2

Integer 2D vector. Used for pixel/drawable dimensions.

```cpp
struct ivec2 { int x = 0, y = 0; };
```

```cpp
glyph::ivec2 size{1920, 1080};
```

!!! note "Phase 5"
    `ivec2` is currently a plain struct. In phase 5 it becomes `using ivec2 = glm::ivec2`, which has the same `.x`/`.y` members plus all glm operations. Existing code will not break.

---

## Upcoming (phase 5)

When glm is added, these aliases will be available in `math.h`:

```cpp
using vec2  = glm::vec2;
using vec3  = glm::vec3;
using vec4  = glm::vec4;
using mat3  = glm::mat3;
using mat4  = glm::mat4;
using ivec2 = glm::ivec2;   // replaces the current struct

float lerp(float a, float b, float t);
vec2  lerp(vec2 a, vec2 b, float t);
```
