# Game & Lifecycle

**Header:** `#include <glyph/app.h>`

---

## AppConfig

Configuration returned by `Game::configure()`. The engine reads this before creating the window.

```cpp
struct AppConfig {
    std::string title      = "Glyph Game";
    int         width      = 1280;
    int         height     = 720;
    bool        vsync      = true;
    bool        resizable  = true;
    bool        fullscreen = false;
};
```

| Field | Default | Description |
|---|---|---|
| `title` | `"Glyph Game"` | Window title bar text |
| `width` | `1280` | Window width in logical pixels |
| `height` | `720` | Window height in logical pixels |
| `vsync` | `true` | Lock frame rate to display refresh |
| `resizable` | `true` | Allow window resize |
| `fullscreen` | `false` | Start in fullscreen mode |

---

## Game

Base class for your game. Subclass it, override the hooks you need, and register with `GLYPH_MAIN`.

```cpp
class Game {
public:
    virtual AppConfig configure();
    virtual void on_start();
    virtual void on_update(float dt);
    virtual void on_render(Renderer& r);
    virtual void on_shutdown();
};
```

### Lifecycle order

```
configure()          ← called before window creation
  ↓
[window + GL context created]
  ↓
on_start()           ← load assets, set up initial state
  ↓
loop:
    on_update(dt)    ← game logic, input (dt = seconds since last frame)
    on_render(r)     ← all draw calls go here
  ↓
on_shutdown()        ← clean up resources
```

### Methods

#### `configure()`
Called once before the window is created. Override to return a custom `AppConfig`.

```cpp
glyph::AppConfig configure() override {
    glyph::AppConfig c;
    c.title = "Pong";
    c.width = 800;
    c.height = 600;
    return c;
}
```

#### `on_start()`
Called once after the window, GL context, and engine subsystems are ready. Load textures and set up initial state here.

```cpp
void on_start() override {
    my_texture_.create(/* ... */);
}
```

#### `on_update(float dt)`
Called every frame. `dt` is the time in seconds since the last frame, capped to prevent spiral-of-death on slow frames.

```cpp
void on_update(float dt) override {
    player_x_ += speed_ * dt;
}
```

#### `on_render(Renderer& r)`
Called every frame after `on_update`. All draw calls must go here. The framebuffer is already cleared to the current `set_clear_color` before this call.

```cpp
void on_render(glyph::Renderer& r) override {
    r.draw_textured_quad(my_texture_, {100, 100, 64, 64});
    r.draw_text(*font_, "Score: 0", {10, 10});
}
```

#### `on_fixed_update(float dt)`
Called zero or more times per frame at a fixed timestep (`Time::kFixedDt` = 1/60 s). Use for physics and deterministic simulation. `dt` is always exactly `kFixedDt`.

#### `on_shutdown()`
Called once before subsystems are torn down. Release any resources or state that requires the GL context.

---

## Engine service accessors

Available from any lifecycle hook (`on_start` onward). Never call from `configure()`.

| Accessor | Type | Description |
|---|---|---|
| `audio()` | `Audio&` | Sound + music playback |
| `input()` | `Input&` | Keyboard, mouse, action bindings |
| `scene()` | `Scene&` | ECS — entities, components, systems |
| `time()` | `Time&` | Frame timing, fixed timestep, FPS |
| `resources()` | `Resources&` | Asset cache (textures, fonts, sounds) |
| `lua()` | `LuaState&` | Lua scripting state (include `<glyph/lua_state.h>`) |

```cpp
void on_start() override {
    auto tex = resources().texture("player.png");
    auto e   = scene().create_entity("player");
    e.add<glyph::Transform>();
    e.add<glyph::Sprite>(glyph::Sprite{tex});
}

void on_update(float dt) override {
    if (input().action_pressed("confirm"))
        audio().play(resources().sound("click.wav"));
}
```

---

## GLYPH_MAIN

Registers your `Game` subclass as the entry point. Call it at file scope in exactly one translation unit.

```cpp
GLYPH_MAIN(MyGame)
```

This expands to:

```cpp
extern "C" glyph::Game* glyph_create_game() { return new MyGame(); }
```

The engine's `main()` (in `main_entry.cpp`) resolves this symbol at link time.

!!! warning
    `GLYPH_MAIN` must appear exactly once across your whole program. Putting it in a header that gets included multiple times will cause a linker error.
