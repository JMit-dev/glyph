# Renderer

**Header:** `#include <glyph/renderer.h>`

The `Renderer` is passed to `Game::on_render()` each frame. All draw calls must happen inside that hook. The renderer is owned by the engine — do not store a pointer to it.

---

## Call order

Every frame, call in this order:

```
begin_frame()
  → clear()             (optional, but recommended)
  → draw_textured_quad() / [future draw calls]
end_frame()
```

The engine calls `begin_frame()` and `end_frame()` for you. Inside `on_render(Renderer& r)`, you only need the draw calls.

---

## Methods

### `clear(Color c)`

Fill the framebuffer with a solid color. Call this at the start of `on_render` to erase the previous frame.

```cpp
void on_render(glyph::Renderer& r) override {
    r.clear(glyph::Color::rgba8(30, 30, 30));  // dark grey
}
```

### `draw_textured_quad(const Texture& tex, Rect dest, Color tint)`

Draw a single textured rectangle. `dest` is in pixel/screen coordinates (y-down, origin top-left).

| Parameter | Type | Description |
|---|---|---|
| `tex` | `const Texture&` | GPU texture to sample from |
| `dest` | `Rect` | Destination rectangle `{x, y, w, h}` in screen pixels |
| `tint` | `Color` | Multiply color. Default: `{1,1,1,1}` (no tint) |

```cpp
// Draw a 64×64 quad at screen position (100, 100)
r.draw_textured_quad(my_texture_, {100.f, 100.f, 64.f, 64.f});

// Draw with a red tint
r.draw_textured_quad(my_texture_, {200.f, 100.f, 64.f, 64.f},
                     glyph::Color::rgba8(255, 100, 100));
```

!!! note "No batching yet"
    Each `draw_textured_quad` call issues one GL draw call. The sprite batcher (phase 4) will collapse many draws into a single GPU submission.

### `set_viewport(int w, int h)`

Update the GL viewport and recalculate the orthographic projection. Call this when the window is resized. The engine calls it automatically during `init()`.

---

## Coordinate system

Glyph uses **Y-down, pixel units**:

- `(0, 0)` is the **top-left** of the window.
- X increases to the right; Y increases downward.
- One unit = one screen pixel at zoom 1.0.
- This matches SDL, Tiled, and most 2D conventions.

```
(0,0) ──────────────→ x
  │
  │
  ↓
  y
```

---

## Thread safety

All `Renderer` methods must be called from the **main thread**. This is the only thread that may issue GL calls.
