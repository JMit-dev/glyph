# Renderer

**Header:** `#include <glyph/renderer.h>`

The `Renderer` is passed to `Game::on_render()` each frame. All draw calls must happen inside that hook (or inside `Scene::render()` which the engine calls automatically). The renderer is owned by the engine — do not store a pointer to it.

---

## Frame lifecycle

The engine manages `begin_frame()` and `end_frame()` for you. `begin_frame()` clears the framebuffer with the current clear color and uploads the camera VP matrix. Inside `on_render(Renderer& r)` you only need draw calls:

```cpp
void on_render(glyph::Renderer& r) override {
    r.draw_textured_quad(my_tex_, {100, 150, 64, 64});
    r.draw_text(*font_, "Score: 42", {10, 20});
}
```

---

## Methods

### `set_clear_color(Color c)`

Set the background color used to wipe the framebuffer at the start of every frame. Default is opaque black.

```cpp
void on_start() override {
    renderer().set_clear_color(glyph::Color::rgba8(30, 30, 60));
}
```

### `clear(Color c)`

Manually fill the framebuffer with a solid color. Rarely needed — `begin_frame()` already clears automatically. If called inside `on_render()`, it wipes any sprites already drawn that frame.

### `draw_textured_quad(tex, dest, tint)`

Draw a single textured rectangle in world space.

| Parameter | Type | Description |
|---|---|---|
| `tex` | `const Texture&` | GPU texture |
| `dest` | `Rect` | Destination `{x, y, w, h}` in world pixels |
| `tint` | `Color` | Multiply color — default `{1,1,1,1}` |

```cpp
r.draw_textured_quad(my_tex_, {100.f, 150.f, 64.f, 64.f});
r.draw_textured_quad(my_tex_, {200.f, 150.f, 64.f, 64.f},
                     glyph::Color::rgba8(255, 100, 100));  // red tint
```

### `draw_textured_quad(tex, dest, src_px, tint)`

Same as above but samples from a sub-region of the texture. `src_px` is a pixel rect in texture space — used internally by `Scene::render()` for sprite sheets and tilemaps.

### `draw_text(font, text, pos, tint)`

Draw a UTF-8 string using a pre-loaded `Font` atlas.

| Parameter | Type | Description |
|---|---|---|
| `font` | `const Font&` | Loaded font (see [`font.h`](font.md)) |
| `text` | `std::string_view` | String to render (ASCII 32–126) |
| `pos` | `vec2` | Pen origin: left edge of the text baseline |
| `tint` | `Color` | Text color — default white `{1,1,1,1}` |

```cpp
r.draw_text(*font_, "Hello!", {10.f, 40.f}, {1, 1, 0, 1});  // yellow
```

### `set_camera(const Camera& c)`

Replace the active camera for the next `begin_frame()`. `Scene::render()` calls this automatically when a `Camera2D` component is found.

### `set_viewport(int w, int h)`

Update the GL viewport and recalculate the camera's viewport size. Called automatically on init; call again on window resize.

### `camera()` / `camera() const`

Access the current camera directly.

---

## Coordinate system

Glyph uses **Y-down, pixel units**:

- `(0, 0)` is the **top-left** of the world.
- X increases right; Y increases downward.
- One world unit = one screen pixel at camera zoom 1.0.
- Matches SDL, Tiled, and most 2D conventions.

```
(0,0) ──────────────→ x
  │
  │
  ↓
  y
```

---

## Thread safety

All `Renderer` methods must be called from the **main thread** — the only thread that may issue GL calls.
