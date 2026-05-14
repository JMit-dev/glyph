# Camera

**Header:** `#include <glyph/camera.h>`

A 2D orthographic camera. Controls what region of world space is visible and at what zoom level. Pass to `Renderer::set_camera()` or modify `Renderer::camera()` directly before draw calls.

---

## Fields

| Field | Type | Default | Description |
|---|---|---|---|
| `position` | `vec2` | `{0, 0}` | World position at the centre of the view |
| `zoom` | `float` | `1.0` | `>1` = zoomed in (things appear larger), `<1` = zoomed out |
| `rotation` | `float` | `0.0` | Radians, clockwise positive (y-down convention) |
| `viewport_size` | `vec2` | `{1280, 720}` | Logical resolution in world units. Updated by `Renderer::set_viewport()`. |

---

## Methods

### `view_projection() → mat4`

Returns the combined orthographic × view matrix for the current camera state. The renderer calls this internally at `begin_frame()` — you rarely need it directly.

```cpp
// Example: passing to a custom shader
mat4 vp = camera.view_projection();
glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(vp));
```

### `screen_to_world(vec2 screen_px) → vec2`

Convert a screen pixel position (top-left origin, y-down) to world space.

```cpp
// Where did the user click in world space?
glyph::vec2 world = r.camera().screen_to_world(mouse_px);
```

### `world_to_screen(vec2 world_pos) → vec2`

Convert a world position to screen pixel coordinates.

```cpp
// Is this entity visible?
glyph::vec2 screen = r.camera().world_to_screen(entity_pos);
bool on_screen = screen.x >= 0 && screen.x < 800
              && screen.y >= 0 && screen.y < 600;
```

---

## Coordinate system

```
World (y-down)          NDC / GL             Screen pixels
(0,0) top-left    →   (-1,+1) top-left  →   (0,0) top-left
  x → right              x → right              x → right
  y ↓ down               y ↑ up                 y ↓ down
```

The camera's `view_projection()` handles all three conversions automatically.

---

## Using with Renderer

```cpp
void on_render(glyph::Renderer& r) override {
    glyph::Camera& cam = r.camera();
    cam.position = player_pos_;          // follow player
    cam.zoom     = 2.f;                  // zoomed in

    r.clear(glyph::Color::rgba8(20, 20, 30));
    // draw calls use cam's VP automatically
    r.draw_textured_quad(sprite_, dest_);
}
```

---

## FitMode

```cpp
enum class FitMode { Stretch, Letterbox, PixelPerfect };
```

Declared for future use. Not yet wired into the renderer — letterboxing and pixel-perfect scaling are deferred until window resize handling is formalised.
