# Lua Scripting

**Header:** `#include <glyph/lua_state.h>`

Glyph embeds Lua 5.4 via [sol2](https://github.com/ThePhD/sol2). Every game has a `LuaState` available through `Game::lua()`. Hot reload is active on desktop — save a script file and the running game picks it up within 250 ms.

---

## LuaState

```cpp
class LuaState {
public:
    bool init(Scene&, Resources&, Audio&, Input&, Time&);
    void shutdown();

    void run_file(const std::string& path);   // execute a Lua file
    void on_update(float dt);                  // calls glyph.on_update(dt) if defined
    void on_render(Renderer&);                 // calls glyph.on_render() if defined

    bool initialized() const;
};
```

Access via `Game::lua()` inside any lifecycle hook.

---

## Embedding Lua in a C++ game

```cpp
void on_start() override {
    lua().run_file("scripts/main.lua");
}
void on_update(float dt) override {
    lua().on_update(dt);   // drives glyph.on_update in Lua
}
void on_render(glyph::Renderer& r) override {
    lua().on_render(r);
}
```

---

## Global Lua hooks (`glyph.*`)

Define any of these in a Lua file loaded with `run_file()`:

```lua
function glyph.on_update(dt)
    -- runs every frame before C++ on_update
end

function glyph.on_render()
    -- runs every frame before C++ on_render
end
```

---

## Entity scripts

Place a Lua module at `scripts/entities/<name>.lua`. Return a table with optional lifecycle methods:

```lua
-- scripts/entities/player.lua
local Player = {}

function Player:on_start()
    self.speed = 200
    self.entity:add_transform({0, 0})
    self.entity:add_sprite("assets/player.png")
end

function Player:on_update(dt)
    local vx = glyph.axis("move_x") * self.speed
    local vy = glyph.axis("move_y") * self.speed
    local t = self.entity:get_transform()
    t.position.x = t.position.x + vx * dt
    t.position.y = t.position.y + vy * dt
end

function Player:on_collision(other)
    -- 'other' is an Entity handle
end

return Player
```

Attach the script from C++:

```cpp
auto e = scene().create_entity("player");
e.add<glyph::Transform>();
e.add<glyph::Script>(glyph::Script{"player"});  // loads scripts/entities/player.lua
```

`ScriptSystem` runs automatically as part of `Scene::run_systems()`:

- **First frame:** loads the module, creates `self`, sets `self.entity`, calls `on_start`.
- **Every frame:** calls `on_update(dt)`.
- **On collision:** calls `on_collision(other)` on both entities in each overlapping `BoxCollider` pair.

---

## `glyph.*` bindings reference

### Math

| Lua | Description |
|---|---|
| `glyph.vec2(x, y)` | 2D float vector with `.x`, `.y` fields and `+`, `-`, `*` operators |
| `glyph.Color(r, g, b, a)` | Linear RGBA color, components 0–1 |

### Scene

| Lua | C++ equivalent |
|---|---|
| `glyph.create_entity()` | `scene().create_entity()` |
| `glyph.create_entity_named(name)` | `scene().create_entity(name)` |
| `glyph.find(name)` | `scene().find(name)` |

### Entity methods (`:` syntax)

| Method | Description |
|---|---|
| `e:valid()` | True if the entity exists |
| `e:destroy()` | Remove entity and all components |
| `e:get_transform()` | Returns `Transform*` or nil |
| `e:get_velocity()` | Returns `Velocity*` or nil |
| `e:get_sprite()` | Returns `Sprite*` or nil |
| `e:add_transform(vec2?)` | Add Transform, optional position |
| `e:add_velocity(vec2?)` | Add Velocity |
| `e:add_sprite(path)` | Add Sprite with texture loaded from path |

### Component fields

**Transform:** `.position` (vec2), `.rotation` (float), `.scale` (vec2)

**Velocity:** `.value` (vec2)

**Sprite:** `.tint` (Color), `.layer` (int), `.visible` (bool)

### Input

| Lua | Description |
|---|---|
| `glyph.key_down(glyph.Key.W)` | True while key is held |
| `glyph.action_down("confirm")` | Named action check |
| `glyph.axis("move_x")` | Axis value −1..1 |

Key constants: `glyph.Key.W/A/S/D`, `Up/Down/Left/Right`, `Space`, `Return`, `Escape`, `LShift`, `LCtrl`, `F1`–`F4`.

### Audio

| Lua | Description |
|---|---|
| `glyph.play_sound(path, vol?)` | Play a sound effect (volume defaults to 1.0) |
| `glyph.play_music(path, vol?)` | Play background music |
| `glyph.stop_music()` | Stop music |

### Resources

| Lua | Description |
|---|---|
| `glyph.load_texture(path)` | Preload texture into cache |

### Time

| Lua | Description |
|---|---|
| `glyph.dt` | Delta time for the current frame (seconds) |
| `glyph.elapsed()` | Total elapsed time (seconds) |

---

## Hot reload

On **desktop**, save any file under `scripts/` and Glyph reloads it within 250 ms:

- **Entity modules** (`scripts/entities/*.lua`): module is re-executed and all live `self` tables have their metatable patched to point at the new class — per-entity state (speed, health, etc.) is preserved.
- **Global scripts** (loaded via `run_file()`): re-executed in place.

Hot reload is **disabled on web and mobile** — scripts are baked into the asset bundle at build time.

!!! tip
    Keep heavy initialisation in `on_start` (runs once) and light per-frame logic in `on_update`. Hot reload replaces methods but not instance variables, so changes to `on_start` only take effect when the entity is recreated.
