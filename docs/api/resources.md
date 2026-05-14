# Resources

**Header:** `#include <glyph/resources.h>`

Reference-counted asset cache. Assets are loaded on demand and cached by resolved path. The cache holds `weak_ptr` references — when all external `shared_ptr`s are released the cache entry expires and is cleaned up on the next `unload_unused()` call.

All methods must be called from the **main thread**.

---

## Quick start

```cpp
class MyGame : public glyph::Game {
    glyph::Resources             res_;
    std::shared_ptr<glyph::Texture> player_;

    void on_start() override {
        res_.set_root("assets");
        player_ = res_.texture("player.png");
    }

    void on_render(glyph::Renderer& r) override {
        r.draw_textured_quad(*player_, {100, 100, 64, 64});
    }

    void on_shutdown() override {
        player_.reset();
        res_.unload_unused();
    }
};
```

---

## Methods

### `set_root(const std::string& path)`

Set the directory prepended to all relative asset paths. Default: `"assets"`.

```cpp
res_.set_root("assets");          // looks in ./assets/
res_.set_root("/absolute/path");  // absolute path
```

### `texture(const std::string& path) → shared_ptr<Texture>`

Load a texture from an image file (PNG, JPG, BMP, TGA via stb_image). Returns a cached copy if the same path was loaded before and is still alive.

Returns `nullptr` and logs an error if the file cannot be read or decoded.

```cpp
auto tex = res_.texture("tiles.png");
if (!tex) { /* handle error */ }

// Second call returns the same GPU texture — no re-upload.
auto tex2 = res_.texture("tiles.png");
assert(tex.get() == tex2.get());
```

### `unload_unused()`

Remove expired cache entries (those whose `shared_ptr`s have all been released). Call this in `on_shutdown()` or periodically when switching scenes.

```cpp
void on_shutdown() override {
    player_.reset();
    res_.unload_unused();   // frees the GPU texture
}
```

---

## Path resolution

| Path | Resolved to |
|---|---|
| `"player.png"` | `{root}/player.png` |
| `"ui/button.png"` | `{root}/ui/button.png` |
| `"/absolute/tex.png"` | `/absolute/tex.png` (unchanged) |
| `"C:\\data\\tex.png"` | `C:\data\tex.png` (unchanged) |

---

## Lifetime

The `Resources` instance does not need to outlive the textures it created — each `shared_ptr<Texture>` is self-contained. However, keeping a single `Resources` per scene (or per game) avoids redundant disk reads.

!!! warning "GL context required"
    `texture()` uploads pixel data to the GPU. Only call it after `on_start()` while the GL context is active.

---

## Coming in future phases

| Method | Phase |
|---|---|
| `sprite_sheet(path)` | 12 — Aseprite JSON |
| `tilemap(path)` | 13 — Tiled TMJ |
| `font(path, size)` | 15 — stb_truetype |
| `sound(path)` | 9 — miniaudio |
| `music(path)` | 9 — miniaudio |
