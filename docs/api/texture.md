# Texture

**Header:** `#include <glyph/texture.h>`

A 2D GPU texture. Created from raw RGBA8 pixel data. In phase 6, `Resources::texture()` will load from PNG/JPG files via stb_image — for now, create textures manually from pixel arrays.

---

## Creating a texture

```cpp
// 2×2 checkerboard (black and white)
uint8_t pixels[2 * 2 * 4] = {
    255, 255, 255, 255,   0,   0,   0, 255,   // row 0: white, black
      0,   0,   0, 255, 255, 255, 255, 255,   // row 1: black, white
};

glyph::Texture tex;
tex.create(2, 2, pixels);
```

!!! warning "GL context required"
    `create()` uploads data to the GPU. Call it after `on_start()` — never in a constructor or before the engine initializes.

---

## Methods

### `create(int w, int h, const uint8_t* rgba_pixels) → bool`

Upload RGBA8 pixels and allocate a GL texture object. Returns `true` on success.

- Pixel format: 4 bytes per pixel, R G B A order, top-row first.
- Filtering: **nearest-neighbor** (sharp pixels, no blurring — ideal for pixel art).
- Wrap mode: `CLAMP_TO_EDGE`.

```cpp
bool ok = tex.create(width, height, pixel_data);
if (!ok) { /* handle error */ }
```

### `destroy()`

Free the GPU texture. Call this in `Game::on_shutdown()` for any textures you created manually.

```cpp
void on_shutdown() override {
    tex_.destroy();
}
```

Calling `destroy()` on an already-destroyed texture is safe (no-op).

### `bind(int slot = 0)`

Bind to a GL texture slot (0–7). You generally don't need to call this directly — draw calls handle binding internally.

### `valid() → bool`

Returns `true` if the texture has been successfully created and not yet destroyed.

### `width() → int` / `height() → int`

Dimensions in pixels.

### `id() → unsigned int`

The raw GL texture name. Exposed for advanced use; prefer the higher-level draw API.

---

## Lifetime

`Texture` is a plain value type — it doesn't auto-destroy when it goes out of scope. Always pair `create()` with a matching `destroy()` call in `on_shutdown()`.

```cpp
class MyGame : public glyph::Game {
    glyph::Texture atlas_;

    void on_start() override    { atlas_.create(/* ... */); }
    void on_shutdown() override { atlas_.destroy(); }
};
```

!!! note "Future: Resources"
    Phase 6 introduces `Resources::texture(path)` which returns a `shared_ptr<Texture>` with automatic lifetime management and caching.
