# Time

**Header:** `#include <glyph/time.h>`

Frame timing and the fixed-timestep accumulator. Access via `time()` inside any `Game` lifecycle hook.

Implements the "Fix Your Timestep" pattern (Glenn Fiedler, 2006): physics and gameplay logic run at a fixed 60 Hz regardless of display frame rate.

---

## Quick start

```cpp
void on_fixed_update(float dt) override {
    // dt is always Time::kFixedDt (1/60 s)
    // Put physics, AI, and frame-rate-sensitive logic here
    velocity_ += gravity_ * dt;
    position_ += velocity_ * dt;
}

void on_update(float dt) override {
    // dt is variable (capped, time-scaled)
    // Use for input reading, camera, and anything tolerant of variable rate
}

void on_render(glyph::Renderer& r) override {
    // Interpolate between physics frames for smooth rendering
    float alpha = time().fixed_alpha();
    glyph::vec2 render_pos = glyph::lerp(prev_pos_, position_, alpha);
    r.draw_textured_quad(sprite_, {render_pos.x, render_pos.y, 32, 32});
}
```

---

## Constants

| Constant | Value | Description |
|---|---|---|
| `Time::kFixedDt` | `1/60 ≈ 0.01667 s` | Fixed update step duration |
| `Time::kMaxAccum` | `0.25 s` | Accumulator cap — prevents spiral of death |

If a frame takes longer than `kMaxAccum`, at most `0.25 / (1/60) = 15` fixed steps run that frame. The simulation slows down rather than running away.

---

## Methods

### `delta() → float`
Time-scaled, capped frame delta in seconds. Always use this in `on_update`, not raw dt.

### `elapsed() → double`
Unscaled wall-clock seconds since the engine started.

### `frame_count() → uint64_t`
Total number of frames rendered.

### `fps() → float`
Exponentially smoothed frame rate (~20-frame window). Suitable for HUD display.

### `fixed_alpha() → float`
Sub-step interpolation factor in `[0, 1]`. How far into the next fixed step the current render is:
- `0.0` = rendering at exactly the previous fixed-update state
- `1.0` = rendering at exactly the current fixed-update state

Use with `lerp()` to eliminate visual jitter when `on_fixed_update` runs slower than the display.

### `set_time_scale(float s)`
Scale all time. `1.0` = normal, `0.5` = half speed (slow motion), `2.0` = double speed. Affects `delta()` and the fixed-step accumulator.

---

## Frame loop sequence

```
time.tick(raw_dt)           — cap, scale, accumulate
while time.step_fixed():    — drain fixed budget (max kMaxAccum / kFixedDt per frame)
    on_fixed_update(1/60)
on_update(time.delta())     — variable-rate
on_render(renderer)         — use fixed_alpha() for interpolation
```

---

## Slow-frame verification

With `kMaxAccum = 0.25 s`, a 2-second freeze causes exactly 15 fixed steps on the next frame, not 120. The game slows to 1/8 speed temporarily instead of running ahead to catch up.
