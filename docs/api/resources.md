# Resources

**Header:** `#include <glyph/resources.h>`

Reference-counted asset cache. Assets are loaded on demand and cached by resolved path. The cache holds `weak_ptr` references — when all external `shared_ptr`s are released, the entry expires and is cleaned up on the next `unload_unused()` call.

All methods must be called from the **main thread**.

---

## Quick start

The engine provides a shared `Resources` instance accessible via `Game::resources()`:

```cpp
class MyGame : public glyph::Game {
    std::shared_ptr<glyph::Texture> player_;
    std::shared_ptr<glyph::Font>    font_;

    void on_start() override {
        player_ = resources().texture("player.png");
        font_   = resources().font("fonts/roboto.ttf", 20);
    }

    void on_render(glyph::Renderer& r) override {
        r.draw_textured_quad(*player_, {100, 100, 64, 64});
        r.draw_text(*font_, "Score: 0", {10, 10});
    }
};
```

---

## Methods

### `set_root(path)` / `root()`

Set (or get) the directory prepended to all relative asset paths. Default: `"assets"`.

```cpp
resources().set_root("assets");   // relative paths look in ./assets/
```

### `texture(path) → shared_ptr<Texture>`

Load a PNG/JPG/BMP/TGA from disk via stb_image. Returns a cached copy if already alive. Returns `nullptr` on failure.

```cpp
auto tex = resources().texture("tiles.png");
// Second call returns the same object — no re-upload.
auto tex2 = resources().texture("tiles.png");
assert(tex.get() == tex2.get());
```

### `font(path, size_px) → shared_ptr<Font>`

Load a TTF/OTF file and bake an ASCII glyph atlas at the given pixel size. Cached by `(path, size_px)`. Returns `nullptr` on failure.

```cpp
auto body = resources().font("fonts/roboto.ttf", 18);
auto title = resources().font("fonts/roboto.ttf", 36);  // separate atlas
```

### `sound(path) → shared_ptr<Sound>`

Returns a `Sound` path-wrapper. Actual audio decoding happens at play time via `Audio::play()`.

```cpp
auto sfx = resources().sound("sfx/jump.wav");
audio().play(sfx);
```

### `music(path) → shared_ptr<Music>`

Returns a `Music` path-wrapper for streaming playback.

```cpp
auto bgm = resources().music("music/theme.ogg");
audio().play_music(bgm);
```

### `unload_unused()`

Remove cache entries whose `shared_ptr`s have all been released. Call when switching scenes or in `on_shutdown()`.

```cpp
void on_shutdown() override {
    player_.reset();
    resources().unload_unused();
}
```

---

## Path resolution

| Path | Resolved as |
|---|---|
| `"player.png"` | `{root}/player.png` |
| `"ui/button.png"` | `{root}/ui/button.png` |
| `"/absolute/path.png"` | unchanged |
| `"C:\\data\\tex.png"` | unchanged (Windows absolute) |

---

## Notes

!!! warning "GL context required"
    `texture()` and `font()` upload data to the GPU. Only call them from `on_start()` or later lifecycle hooks while the GL context is active — never from `configure()`.

!!! tip "Lua access"
    From Lua scripts, use `glyph.load_texture(path)` to preload into the shared cache, or `glyph.play_sound(path)` / `glyph.play_music(path)` which resolve through Resources automatically.
