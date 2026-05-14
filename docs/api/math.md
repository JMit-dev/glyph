# Math Types

**Header:** `#include <glyph/math.h>`

Core math types and utilities. `vec2`, `vec3`, `mat4`, etc. are aliases over [glm](https://github.com/g-truc/glm) so game code can use the full glm API while staying in the `glyph` namespace.

---

## Type aliases

```cpp
using vec2  = glm::vec2;
using vec3  = glm::vec3;
using vec4  = glm::vec4;
using mat3  = glm::mat3;
using mat4  = glm::mat4;
using ivec2 = glm::ivec2;
```

Because these are glm aliases, all glm operations work directly:

```cpp
glyph::vec2 a{100.f, 200.f};
glyph::vec2 b{300.f, 400.f};

float  dist = glm::length(b - a);
glyph::vec2 norm = glm::normalize(b - a);
glyph::mat4 rot  = glm::rotate(glm::mat4(1.f), 0.5f, glyph::vec3(0, 0, 1));
```

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

Convert 8-bit values (0–255) to a `Color`. The `a` parameter defaults to 255 (fully opaque).

```cpp
glyph::Color red       = glyph::Color::rgba8(255,   0,   0);
glyph::Color sky_blue  = glyph::Color::rgba8(100, 149, 237);
glyph::Color half_alpha= glyph::Color::rgba8(255, 255, 255, 128);
```

### Direct construction

```cpp
glyph::Color white{};              // {1, 1, 1, 1} — default
glyph::Color black{0, 0, 0, 1};
glyph::Color transparent{1,1,1,0};
```

---

## Rect

Axis-aligned rectangle in float coordinates (y-down, pixel units).

```cpp
struct Rect {
    float x, y, w, h;

    bool contains(vec2 p) const;
    bool intersects(Rect o) const;
};
```

| Field | Description |
|---|---|
| `x`, `y` | Top-left corner |
| `w`, `h` | Width and height |

```cpp
glyph::Rect screen{0.f, 0.f, 1280.f, 720.f};
glyph::Rect tile  {64.f, 32.f, 16.f, 16.f};

bool hit = tile.contains({70.f, 40.f});   // true
bool overlap = screen.intersects(tile);    // true
```

---

## ivec2

Integer 2D vector — an alias for `glm::ivec2`.

```cpp
using ivec2 = glm::ivec2;
```

```cpp
glyph::ivec2 drawable = window.drawable_size();  // {1920, 1080}
int area = drawable.x * drawable.y;
```

---

## lerp

```cpp
float lerp(float a, float b, float t);
vec2  lerp(vec2  a, vec2  b, float t);
```

Linear interpolation. `t = 0` returns `a`, `t = 1` returns `b`.

```cpp
float mid  = glyph::lerp(0.f, 100.f, 0.5f);   // 50.0
glyph::vec2 pos = glyph::lerp(start, end, 0.25f);
```
