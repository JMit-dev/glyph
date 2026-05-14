# Input

**Header:** `#include <glyph/input.h>`

Access via `input()` inside any `Game` lifecycle hook. The engine updates state from SDL events before each frame.

---

## Quick start

```cpp
void on_update(float dt) override {
    // Axis — returns -1, 0, or +1 (clamped, normalised across all bound pairs)
    float vx = input().axis_value("move_x");   // A/D + Left/Right by default
    float vy = input().axis_value("move_y");   // W/S + Up/Down by default

    // Single-frame press detection
    if (input().action_pressed("confirm"))     // Space or Enter by default
        start_action();

    // Raw key query
    if (input().key_down(glyph::Key::LShift))
        speed *= 2.f;

    // Mouse
    glyph::vec2 cursor = input().mouse_position();
    float scroll = input().mouse_wheel().y;
}
```

---

## Keyboard

### `key_down(Key k) → bool`
True while the key is held.

### `key_pressed(Key k) → bool`
True **only on the frame** the key went from up to down.

### `key_released(Key k) → bool`
True **only on the frame** the key went from down to up.

### Key enum

Common values (full list in `input.h`):

| Enum | Key |
|---|---|
| `Key::A`–`Key::Z` | Letter keys |
| `Key::Num0`–`Key::Num9` | Number row |
| `Key::Up/Down/Left/Right` | Arrow keys |
| `Key::Space`, `Key::Return`, `Key::Escape` | Special |
| `Key::LShift`, `Key::LCtrl`, `Key::LAlt` | Modifiers |
| `Key::F1`–`Key::F12` | Function keys |

---

## Mouse

### `mouse_position() → vec2`
Current cursor position in screen pixels (top-left origin).

### `mouse_down(MouseButton) → bool` / `mouse_pressed` / `mouse_released`
Same frame-delta semantics as keyboard.

### `mouse_wheel() → vec2`
Accumulated scroll delta this frame. `y > 0` = scroll up. Resets to zero each frame.

### MouseButton enum
`MouseButton::Left`, `MouseButton::Right`, `MouseButton::Middle`

---

## Action bindings

Named bindings decouple game logic from specific keys.

### `bind_action(name, Key)`
Add a key trigger to a named action. Multiple keys can be bound; any one fires the action.

```cpp
input().bind_action("jump", glyph::Key::Space);
input().bind_action("jump", glyph::Key::Up);    // second binding for same action
```

### `bind_axis(name, Key neg, Key pos)`
Add a key pair to a named axis. Multiple pairs are OR'd and clamped to `[-1, 1]`.

```cpp
input().bind_axis("move_x", glyph::Key::Left, glyph::Key::Right);
input().bind_axis("move_x", glyph::Key::A,    glyph::Key::D);   // both pairs active
```

### `action_down(name)` / `action_pressed(name)` → bool
`action_pressed` returns true only on the frame any bound key is first pressed.

### `axis_value(name)` → float
Returns `−1`, `0`, or `+1` (clamped). Diagonal movement should be normalised by the caller.

---

## Default bindings

Set up in `Input::Input()`. Games can add more or override with `bind_axis`/`bind_action`.

| Name | Keys |
|---|---|
| `"move_x"` | `A`/`D` + `Left`/`Right` |
| `"move_y"` | `W`/`S` + `Up`/`Down` |
| `"confirm"` | `Space`, `Return` |
| `"cancel"` | `Escape` |

---

## Frame timing

```
SDL events fired  →  SDL_AppEvent updates Input state
         ↓
SDL_AppIterate:  on_update() + on_render()  ←  game reads Input here
         ↓
Input::begin_frame()  — copies current→previous, resets mouse_wheel
```

`key_pressed` is true for exactly one frame per physical key press.
