# Glyph — 2D Game Engine Specification

A code-first, cross-platform 2D game engine. C++17 core, Lua 5.4 scripting layer. Targets Windows, macOS, Linux, Web (WebAssembly), iOS, Android from a single codebase. No GUI editor — games are authored in code (C++ or Lua) using Tiled for levels and Aseprite (or similar) for sprites.

This document is the implementation spec. Read it top to bottom before generating code.

---

## 1. Design principles

1. **Code is the editor.** No bundled GUI tool. The engine exposes a clean C++ API and mirrors it in Lua. Levels come from Tiled (`.tmj` JSON), sprites from Aseprite (`.json` + PNG).
2. **One codebase, every platform.** Anything platform-specific lives behind an interface in `platform/`. Game code never touches `#ifdef _WIN32`.
3. **Single-threaded main loop, by default.** Concurrency is opt-in for asset loading. v1 ships single-threaded.
4. **Data-oriented core, friendly façade.** EnTT drives the ECS internally; the public `Entity` handle hides the registry from users.
5. **Hot reload Lua, always.** Compile cycles kill iteration. Lua scripts re-load on file change.
6. **Boring tech.** SDL3, OpenGL 3.3 / GLES 3.0 / WebGL2, single-header libraries. No bleeding edge.
7. **Y-down, top-left origin, pixel units.** Matches Tiled, SDL, screen coordinates, and most 2D conventions. World units = pixels by default; the camera has a `zoom` scalar.

---

## 2. Tech stack

| Concern | Library | Version | Why |
|---|---|---|---|
| Build system | CMake | 3.20+ | Cross-platform standard |
| Language | C++17 | — | Universally supported, no module headaches |
| Platform/window/input | SDL3 | 3.2+ | Best cross-platform layer, mobile + web support |
| GL loader | glad2 | latest | Loads GL function pointers on desktop |
| Math | glm | 1.0+ | Header-only, GLSL-style, battle-tested |
| Image loading | stb_image | latest | Single header, PNG/JPG/etc |
| Font rasterizing | stb_truetype | latest | Single header TTF |
| Audio | miniaudio | 0.11+ | Single header, every platform |
| JSON | nlohmann/json | 3.11+ | Header-only, Tiled and Aseprite output JSON |
| ECS | EnTT | 3.13+ | Fast, header-only, well-documented |
| Scripting | Lua | 5.4 | Plain Lua, not LuaJIT (LuaJIT doesn't run on iOS/WASM) |
| C++ ↔ Lua | sol2 | 3.3+ | Cleanest Lua binding library |
| Logging | spdlog | 1.13+ | Fast, header-only mode available |
| Physics (optional) | Box2D | 3.0+ | Only if you need real physics; AABB is built-in |

All dependencies vendored as git submodules under `external/`. No system packages required.

---

## 3. Coordinate system and conventions

- **Y-down, X-right.** Origin (0, 0) is the top-left of the world.
- **Pixel units.** A world unit equals one screen pixel at camera zoom 1.0.
- **Rotation:** radians, clockwise positive (because Y is down).
- **Color:** linear `vec4` 0..1 in code; sRGB textures are decoded on load.
- **Transforms:** translation, rotation, scale — applied in TRS order.
- **Z-order:** integer `layer` on `Sprite` component (lower = drawn first). Within a layer, sort by Y for top-down games is opt-in.
- **Time:** seconds, `float` for delta time, `double` for accumulated time.
- **Strings:** UTF-8 throughout.

---

## 4. Directory layout

```
glyph/
├── CMakeLists.txt
├── cmake/                      # toolchain files for emscripten, android, ios
├── external/                   # submodules: SDL, glm, EnTT, sol2, lua, stb, etc.
├── include/glyph/              # public headers (the API users see)
│   ├── glyph.h                 # umbrella header
│   ├── app.h                   # Application/Game base class
│   ├── entity.h
│   ├── scene.h
│   ├── components.h            # Transform, Sprite, Animator, Collider, etc.
│   ├── renderer.h
│   ├── camera.h
│   ├── texture.h
│   ├── sprite_sheet.h
│   ├── tilemap.h
│   ├── audio.h
│   ├── input.h
│   ├── math.h                  # vec2, rect, color aliases over glm
│   ├── resources.h
│   ├── time.h
│   ├── log.h
│   ├── lua_api.h
│   └── platform.h
├── src/                        # implementation
│   ├── core/
│   │   ├── app.cpp
│   │   ├── time.cpp
│   │   ├── log.cpp
│   │   └── resources.cpp
│   ├── platform/
│   │   ├── sdl_platform.cpp
│   │   ├── window.cpp
│   │   └── file_io.cpp
│   ├── renderer/
│   │   ├── gl.h                # GL/GLES/WebGL abstraction
│   │   ├── renderer.cpp
│   │   ├── sprite_batch.cpp
│   │   ├── shader.cpp
│   │   ├── texture.cpp
│   │   └── shaders/            # GLSL source as embedded strings
│   ├── audio/
│   │   └── audio.cpp           # wraps miniaudio
│   ├── input/
│   │   └── input.cpp
│   ├── scene/
│   │   ├── scene.cpp
│   │   ├── entity.cpp
│   │   └── systems.cpp         # built-in systems: render, animate, collide
│   ├── content/
│   │   ├── sprite_sheet.cpp    # Aseprite JSON parser
│   │   ├── tilemap.cpp         # Tiled TMJ parser
│   │   └── font.cpp
│   ├── scripting/
│   │   ├── lua_state.cpp
│   │   ├── lua_bindings.cpp    # sol2 type registrations
│   │   └── lua_hotreload.cpp
│   └── main_entry.cpp          # GLYPH_MAIN() macro expansion
├── samples/
│   ├── 01_hello_window/
│   ├── 02_sprite/
│   ├── 03_input/
│   ├── 04_tilemap/
│   └── 05_lua_platformer/
├── docs/
└── tests/
```

---

## 5. Build system (CMake)

Top-level `CMakeLists.txt` defines:

- `glyph` static library target (links the engine).
- `glyph_runtime` executable target (entry point, links `glyph` and a chosen game).
- `GLYPH_BUILD_SAMPLES` option (default ON).
- `GLYPH_BUILD_TESTS` option (default OFF).
- Platform detection sets `GLYPH_PLATFORM_DESKTOP|WEB|ANDROID|IOS`.

Per platform:
- **Desktop:** standard executable, links SDL3 statically.
- **Web:** Emscripten toolchain file sets `-s USE_WEBGL2=1 -s FULL_ES3=1 -s ALLOW_MEMORY_GROWTH=1 --preload-file assets`. Output `.html`/`.js`/`.wasm`.
- **Android:** Gradle wraps the CMake build via `externalNativeBuild`. SDL3 has an Android activity that calls into native `SDL_main`.
- **iOS:** CMake generates an Xcode project; SDL3 provides the bootstrap.

Compiler flags: `-Wall -Wextra -Wpedantic`, `-fno-exceptions` is **off** (sol2 uses exceptions), `-fno-rtti` is **off** (EnTT and sol2 use it). `-O2` for release, `-O0 -g` for debug.

---

## 6. Application lifecycle

The engine owns `main()`. Users subclass `glyph::Game` and register it via `GLYPH_MAIN(MyGame)`.

```cpp
// glyph/app.h
namespace glyph {

struct AppConfig {
    std::string title    = "Glyph Game";
    int         width    = 1280;
    int         height   = 720;
    bool        vsync    = true;
    bool        resizable = true;
    bool        fullscreen = false;
    std::string asset_root = "assets";
    std::string boot_script;            // optional Lua file run on start
};

class Game {
public:
    virtual ~Game() = default;
    virtual AppConfig configure() { return {}; }
    virtual void on_start()             {}
    virtual void on_update(float dt)    {}
    virtual void on_fixed_update(float dt) {}
    virtual void on_render(Renderer&)   {}
    virtual void on_shutdown()          {}

    // Services accessible to subclasses:
    Scene&        scene();
    Renderer&     renderer();
    Input&        input();
    Audio&        audio();
    Resources&    resources();
    Time&         time();
    LuaState&     lua();
};

} // namespace glyph

#define GLYPH_MAIN(GameClass) \
    glyph::Game* glyph_create_game() { return new GameClass(); }
```

Internal `main()` (in `main_entry.cpp`):

```
init platform (SDL, window, GL context)
init subsystems (renderer, audio, input, resources, scene, lua)
game = glyph_create_game();
apply game->configure()
game->on_start();
loop {
    poll events; update input
    accumulate fixed-step time
    while accumulator >= FIXED_DT: game->on_fixed_update(FIXED_DT); accumulator -= FIXED_DT
    game->on_update(dt)
    scene.run_systems(dt)               // animations, physics, etc.
    renderer.begin_frame();
    scene.render(renderer);
    game->on_render(renderer);
    renderer.end_frame();
    present
}
game->on_shutdown();
teardown subsystems
```

Fixed step constant: `FIXED_DT = 1.0f / 60.0f`. Cap the accumulator at `0.25s` to prevent the spiral of death. See *Fix Your Timestep* by Glenn Fiedler — implement that pattern verbatim.

**Web platform note:** Emscripten can't block the main thread. Wrap the loop body in a function and call `emscripten_set_main_loop(loop_fn, 0, 1)` instead of a `while`.

---

## 7. Math

`glyph/math.h` aliases glm and adds small utilities.

```cpp
namespace glyph {
using vec2 = glm::vec2;
using vec3 = glm::vec3;
using vec4 = glm::vec4;
using mat3 = glm::mat3;
using mat4 = glm::mat4;
using ivec2 = glm::ivec2;

struct Rect { float x, y, w, h; bool contains(vec2 p) const; bool intersects(Rect o) const; };
struct Color { float r, g, b, a; static Color rgba8(uint8_t,uint8_t,uint8_t,uint8_t); };

float lerp(float a, float b, float t);
vec2  lerp(vec2 a, vec2 b, float t);
}
```

No custom SIMD types. glm is fast enough for 2D.

---

## 8. Logging

`spdlog` in header-only mode. Four macros:
```
GLYPH_TRACE(fmt, ...)
GLYPH_INFO(fmt, ...)
GLYPH_WARN(fmt, ...)
GLYPH_ERROR(fmt, ...)
```
Compile-time stripped below the configured level in release builds. Output to stderr on desktop, `__android_log_print` on Android, `EM_ASM_({console.log...})` on web.

---

## 9. Platform layer

`platform/sdl_platform.cpp` is the only file allowed to include `<SDL3/SDL.h>`. It exposes:

```cpp
class Window {
public:
    bool create(const AppConfig&);
    void destroy();
    void swap_buffers();
    ivec2 size() const;
    ivec2 drawable_size() const;     // may differ on HiDPI
    float dpi_scale() const;
    SDL_Window* sdl_handle();        // internal use only
};

class FileIO {
public:
    static std::vector<uint8_t> read_binary(const std::string& path);
    static std::string          read_text(const std::string& path);
    static bool                 exists(const std::string& path);
};
```

On web/mobile, asset paths are routed through SDL's virtual filesystem (`SDL_GetBasePath`, Android's APK assets, Emscripten's preloaded FS).

---

## 10. Resource manager

Reference-counted handles. Resources cached by path.

```cpp
class Resources {
public:
    std::shared_ptr<Texture>     texture(const std::string& path);
    std::shared_ptr<SpriteSheet> sprite_sheet(const std::string& aseprite_json);
    std::shared_ptr<Tilemap>     tilemap(const std::string& tmj);
    std::shared_ptr<Font>        font(const std::string& ttf, int size_px);
    std::shared_ptr<Sound>       sound(const std::string& path);
    std::shared_ptr<Music>       music(const std::string& path);

    void unload_unused();   // drops cache entries with no external refs
    void set_root(const std::string&);   // prefix for relative paths
};
```

Loaders use `FileIO::read_binary` so the same path syntax works on every platform.

---

## 11. Renderer

### 11.1 GL abstraction

`renderer/gl.h` selects the right headers:
```
#if defined(__EMSCRIPTEN__) || defined(__ANDROID__) || defined(GLYPH_IOS)
    #include <GLES3/gl3.h>
#else
    #include <glad/gl.h>
#endif
```

Shaders are written once in GLSL ES 3.00 (`#version 300 es`) which both WebGL2 and desktop GL 3.3 accept after a precision-qualifier header injection step.

### 11.2 Renderer interface

```cpp
class Renderer {
public:
    void begin_frame();
    void end_frame();

    void set_camera(const Camera&);
    Camera& camera();

    void clear(Color);

    // immediate-mode 2D API (all calls go through the batcher)
    void draw_sprite(const Sprite&, const Transform&);
    void draw_quad(Rect, Color);
    void draw_line(vec2 a, vec2 b, Color, float thickness = 1.0f);
    void draw_text(const Font&, std::string_view, vec2 pos, Color);

    // raw access for advanced users
    void submit(const Texture&, const Quad& q);  // q = 4 verts in world space
};
```

### 11.3 Sprite batcher

The core of 2D performance. Algorithm:

1. Maintain one big dynamic VBO (e.g. 64 KB, ~2700 sprites). Vertex layout: `vec2 pos, vec2 uv, vec4 color, float tex_index`.
2. Maintain a list of currently bound textures (up to `GL_MAX_TEXTURE_IMAGE_UNITS`, but use 8–16 for portability).
3. On `submit(texture, quad)`:
   - If `texture` is not in the current bind list and the list is full → **flush**.
   - If adding 6 indices would overflow the index buffer → **flush**.
   - Otherwise, append 4 verts + 6 indices, with `tex_index` set to the slot.
4. **Flush** = bind all active textures to their slots, upload vert/index data with `glBufferSubData`, issue one `glDrawElements`.
5. Sort submissions by `(layer, texture)` before draw if user enables sorting. Default: insertion order.

Fragment shader samples from an array of samplers indexed by `tex_index` (the trick that makes multi-texture batching work in one draw):
```glsl
uniform sampler2D u_textures[8];
in float v_tex_index;
void main() {
    int idx = int(v_tex_index);
    vec4 c = texture(u_textures[idx], v_uv) * v_color;
    // ...
}
```
Note: GLES requires the index to be a constant expression in some drivers — emulate with a `switch` if you hit issues.

### 11.4 Camera

```cpp
class Camera {
public:
    vec2 position{0, 0};
    float zoom = 1.0f;
    float rotation = 0.0f;
    vec2 viewport_size{1280, 720};   // logical resolution

    mat4 view_projection() const;     // ortho × view
    vec2 screen_to_world(vec2) const;
    vec2 world_to_screen(vec2) const;
};
```

Use a fixed logical resolution and letterbox/pillarbox to fit the window — keeps games looking consistent across screen sizes. Provide `Camera::set_logical_size(w, h)` and a "fit" mode enum (`Stretch`, `Letterbox`, `PixelPerfect`).

---

## 12. Audio

Thin wrapper over miniaudio. Two abstractions:
- **Sound**: short, fully decoded into memory, low-latency playback. Many simultaneous instances.
- **Music**: streamed from disk, one or two simultaneous (cross-fade support).

```cpp
class Audio {
public:
    bool init();
    void shutdown();

    SoundHandle play(const std::shared_ptr<Sound>&, float volume=1.0f, float pitch=1.0f, bool looped=false);
    void        stop(SoundHandle);
    void        set_volume(SoundHandle, float);

    void play_music(const std::shared_ptr<Music>&, float fade_in_seconds=0.0f);
    void stop_music(float fade_out_seconds=0.0f);

    void master_volume(float);
};
```

miniaudio runs its own playback thread internally. Don't manage threads yourself.

---

## 13. Input

Three layers:

1. **Raw state** — current key/button/axis values from SDL events.
2. **Actions** — named bindings that abstract over device.
3. **Touch/gamepad/keyboard** unified through actions.

```cpp
class Input {
public:
    bool key_down(Key) const;
    bool key_pressed(Key) const;   // this frame
    bool key_released(Key) const;
    vec2 mouse_position() const;
    bool mouse_down(MouseButton) const;
    vec2 mouse_wheel() const;

    // actions
    void bind_action(const std::string& name, Key);
    void bind_action(const std::string& name, GamepadButton);
    void bind_axis(const std::string& name, Key neg, Key pos);
    void bind_axis(const std::string& name, GamepadAxis);

    bool  action_down(const std::string&) const;
    bool  action_pressed(const std::string&) const;
    float axis_value(const std::string&) const;

    // touch (mobile)
    int   touch_count() const;
    vec2  touch_position(int index) const;
};
```

Default action bindings set up by the engine: `move_x` axis (A/D + left stick X), `move_y` axis (W/S + left stick Y), `confirm` (Space + Enter + A button), `cancel` (Escape + B button). Games override freely.

---

## 14. Time

```cpp
class Time {
public:
    float  delta() const;          // last frame, capped
    double elapsed() const;        // since start
    uint64_t frame_count() const;
    float  fps() const;            // smoothed
    void   set_time_scale(float);  // slow-mo
};
```

Fixed timestep accumulator lives inside `Time`. `Time::fixed_alpha()` returns the interpolation factor for rendering between two fixed-update states (use it for smooth physics).

---

## 15. Scene and ECS

EnTT under the hood. `Entity` is a value-typed handle (registry pointer + entt::entity id).

```cpp
class Entity {
    Scene* scene_;
    entt::entity id_;
public:
    bool valid() const;
    void destroy();

    template<class T, class... Args> T& add(Args&&...);
    template<class T> T& get();
    template<class T> T* try_get();
    template<class T> bool has() const;
    template<class T> void remove();
};

class Scene {
public:
    Entity create_entity();
    Entity create_entity(const std::string& name);
    Entity find(const std::string& name);

    template<class... Comps, class Fn>
    void each(Fn&& fn);

    void run_systems(float dt);      // engine-built-in systems
    void render(Renderer&);

    void clear();
    void load_from_tilemap(const Tilemap&);

    entt::registry& registry();      // escape hatch
};
```

### 15.1 Built-in components (`include/glyph/components.h`)

```cpp
struct Name      { std::string value; };
struct Transform { vec2 position; float rotation = 0; vec2 scale{1,1}; Entity parent; };
struct Sprite    { std::shared_ptr<Texture> texture; Rect src{}; Color tint{1,1,1,1}; int layer=0; vec2 origin{0.5f,0.5f}; bool visible=true; };
struct Animator  { std::shared_ptr<SpriteSheet> sheet; std::string clip; float time=0; bool playing=true; bool loop=true; };
struct Velocity  { vec2 value; };
struct BoxCollider { Rect bounds; uint32_t layer=1; uint32_t mask=0xFFFFFFFF; bool is_trigger=false; };
struct TilemapRef { std::shared_ptr<Tilemap> map; };
struct Camera2D  { Camera value; bool primary=true; };
struct Script    { std::string lua_module; sol::table self; };   // a Lua-driven entity
struct Lifetime  { float seconds; };                              // auto-destroy after N seconds
```

### 15.2 Built-in systems (`scene/systems.cpp`)

Run by `Scene::run_systems(dt)` in this order:

1. **LifetimeSystem** — decrement `Lifetime::seconds`; destroy at ≤ 0.
2. **ScriptSystem** — for each `Script`, call its `update(dt)` Lua function.
3. **AnimatorSystem** — advance frame time, update `Sprite::src` from current frame.
4. **MovementSystem** — apply `Velocity * dt` to `Transform::position`.
5. **CollisionSystem** — broad-phase: spatial hash grid (cell size ≈ avg sprite size). Narrow-phase: AABB overlap. Emit `CollisionEvent`s to subscribers. Respect `layer`/`mask`.
6. **TransformPropagationSystem** — resolve parent/child world transforms.

`Scene::render(Renderer&)` then:

1. Find the primary `Camera2D` and apply it.
2. Iterate `Sprite + Transform`, sort by `layer` (and optionally Y), submit to batcher.
3. Render any `TilemapRef`.

---

## 16. Sprite sheets (Aseprite JSON)

Aseprite exports a PNG plus a JSON sidecar. The "Hash" format is preferred.

```cpp
struct AnimClip {
    std::string name;
    std::vector<int> frame_indices;
    std::vector<float> durations;     // seconds, parallel to frame_indices
    bool loop = true;
};

class SpriteSheet {
public:
    std::shared_ptr<Texture> texture;
    std::vector<Rect> frames;                  // frame rects in pixels
    std::unordered_map<std::string, AnimClip> clips;

    static std::shared_ptr<SpriteSheet> load(const std::string& json_path);
};
```

Parsing notes: Aseprite tags map to clip names. Frame `duration` is in milliseconds in the JSON — convert to seconds.

---

## 17. Tilemaps (Tiled TMJ)

Tiled exports JSON with `.tmj` extension. Support the subset most games need; ignore the rest gracefully.

```cpp
struct TileLayer {
    std::string name;
    ivec2 size;                       // in tiles
    std::vector<uint32_t> gids;       // tile global IDs, row-major
    bool visible = true;
    float opacity = 1.0f;
};

struct ObjectLayer {
    std::string name;
    std::vector<TiledObject> objects;
};

struct TiledObject {
    int id;
    std::string name;
    std::string type;                 // user-defined class
    Rect bounds;
    std::unordered_map<std::string, TiledProperty> properties;
};

struct Tileset {
    std::shared_ptr<Texture> texture;
    int tile_w, tile_h;
    int columns;
    int first_gid;
    int tile_count;
    // collision shapes per local tile id, etc.
};

class Tilemap {
public:
    int map_w, map_h;
    int tile_w, tile_h;
    std::vector<Tileset> tilesets;
    std::vector<TileLayer> tile_layers;
    std::vector<ObjectLayer> object_layers;

    static std::shared_ptr<Tilemap> load(const std::string& tmj_path);
    Rect tile_uv(uint32_t gid) const;
    Tileset* tileset_for(uint32_t gid);
};
```

Rendering: one quad per non-zero tile, batched by tileset texture. For very large maps, chunk into screen-sized tiles and frustum-cull.

Loading: `Scene::load_from_tilemap` walks object layers and spawns entities. Convention: an object's `type` field names a Lua module under `scripts/entities/<type>.lua`, which gets instantiated as a `Script` component.

---

## 18. Fonts

`stb_truetype` rasterizes glyphs into a single atlas texture at a given pixel size. Cache `(font_path, size)` → atlas in `Resources`.

```cpp
class Font {
public:
    std::shared_ptr<Texture> atlas;
    struct Glyph { Rect uv; vec2 size; vec2 bearing; float advance; };
    std::unordered_map<uint32_t, Glyph> glyphs;   // by codepoint
    int size_px;
    float line_height;

    vec2 measure(std::string_view text) const;
    static std::shared_ptr<Font> load(const std::string& ttf_path, int size_px);
};
```

ASCII range (32–126) baked at load; non-ASCII codepoints rasterized on first use and inserted into a dynamic atlas page.

---

## 19. Lua scripting layer

### 19.1 Lifecycle

`LuaState` owns one `sol::state` plus a watcher thread for hot reload.

```cpp
class LuaState {
public:
    bool init(Scene& scene, Resources& resources, Audio& audio, Input& input);
    void shutdown();

    void run_file(const std::string& path);
    sol::object require(const std::string& module);   // glyph's own loader, hot-reload aware

    void on_update(float dt);                  // calls global glyph.on_update if present
    void on_render(Renderer&);                 // calls global glyph.on_render if present

    sol::state& state();
};
```

### 19.2 Bindings (sol2)

Bind in `lua_bindings.cpp`. Surface mirrors the C++ API but stays handle-flat (no inheritance hierarchies, no raw pointers).

```cpp
sol::state& L = lua.state();
auto glyph_tbl = L.create_named_table("glyph");

// math
glyph_tbl.new_usertype<vec2>("vec2",
    sol::constructors<vec2(), vec2(float, float)>(),
    "x", &vec2::x, "y", &vec2::y);

// Entity
glyph_tbl.new_usertype<Entity>("Entity",
    "valid",   &Entity::valid,
    "destroy", &Entity::destroy,
    "get_transform", [](Entity& e) -> Transform* { return e.try_get<Transform>(); },
    "add_sprite",    [](Entity& e, const std::string& tex) {
         auto& r = current_app().resources();
         e.add<Sprite>(Sprite{ r.texture(tex) });
    },
    /* ... */);

// Scene shortcuts
glyph_tbl["create_entity"] = [](){ return current_app().scene().create_entity(); };
glyph_tbl["find"]          = [](const std::string& n){ return current_app().scene().find(n); };

// Input
glyph_tbl["key_down"]      = [](int k){ return current_app().input().key_down((Key)k); };
glyph_tbl["action_down"]   = [](const std::string& a){ return current_app().input().action_down(a); };
glyph_tbl["axis"]          = [](const std::string& a){ return current_app().input().axis_value(a); };

// Audio
glyph_tbl["play_sound"]    = [](const std::string& p, float v) {
    auto s = current_app().resources().sound(p);
    return current_app().audio().play(s, v);
};

// Resources
glyph_tbl["load_texture"]  = [](const std::string& p){ current_app().resources().texture(p); };

// Time
glyph_tbl["dt"]            = sol::var(std::ref(global_dt));
glyph_tbl["elapsed"]       = []{ return current_app().time().elapsed(); };
```

### 19.3 Entity scripts

A Lua "entity script" is a module returning a table with optional lifecycle methods:

```lua
-- scripts/entities/player.lua
local Player = {}

function Player:on_start()
    self.speed = 200
    self.entity:add_sprite("player.png")
end

function Player:on_update(dt)
    local vx = glyph.axis("move_x") * self.speed
    local vy = glyph.axis("move_y") * self.speed
    local t = self.entity:get_transform()
    t.position.x = t.position.x + vx * dt
    t.position.y = t.position.y + vy * dt
end

function Player:on_collision(other)
    -- ...
end

return Player
```

The `ScriptSystem` constructs the per-entity `self` table on first update, sets `self.entity`, calls `on_start`, then calls `on_update(dt)` each frame. `on_collision(other)` is invoked from the `CollisionSystem`.

### 19.4 Hot reload

`lua_hotreload.cpp` polls modification times of files under `scripts/` every 250 ms (or uses inotify/FSEvents/ReadDirectoryChanges if you want to get fancy). On change:
1. Clear `package.loaded[modname]`.
2. Re-`require` the module.
3. Patch existing entity `self` tables' metatables to point at the new module table — methods update in place, state preserved.

On web/mobile, hot reload is disabled (filesystem semantics differ); scripts are baked into the asset bundle.

---

## 20. Sample game (Lua)

`samples/05_lua_platformer/main.lua`:

```lua
function glyph.on_start()
    glyph.load_texture("player.png")
    local map = glyph.load_tilemap("levels/world1.tmj")
    glyph.scene_from_tilemap(map)

    local cam = glyph.create_entity("camera")
    cam:add_camera({ zoom = 2.0, primary = true })

    glyph.bind_axis("move_x", glyph.Key.A, glyph.Key.D)
    glyph.bind_action("jump",   glyph.Key.Space)
end

function glyph.on_update(dt)
    local p = glyph.find("player")
    if not p then return end

    local cam = glyph.find("camera"):get_camera()
    local pt  = p:get_transform()
    cam.position.x = pt.position.x
    cam.position.y = pt.position.y
end
```

`scripts/entities/player.lua` handles the player as in §19.3.

---

## 21. C++ sample for comparison

```cpp
#include <glyph/glyph.h>

using namespace glyph;

class Pong : public Game {
    Entity ball, paddle_l, paddle_r;
public:
    AppConfig configure() override {
        AppConfig c; c.title = "Pong"; c.width = 800; c.height = 600; return c;
    }

    void on_start() override {
        auto tex = resources().texture("paddle.png");
        paddle_l = scene().create_entity("paddle_l");
        paddle_l.add<Transform>(Transform{ {50, 300} });
        paddle_l.add<Sprite>(Sprite{ tex });

        // ... ball, paddle_r similar
    }

    void on_update(float dt) override {
        auto& t = paddle_l.get<Transform>();
        t.position.y += input().axis_value("move_y") * 300.0f * dt;
    }
};

GLYPH_MAIN(Pong)
```

---

## 22. Platform-specific build notes

### Desktop (Windows / macOS / Linux)
```
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
./build/glyph_runtime
```

### Web (Emscripten)
```
emcmake cmake -B build-web -DCMAKE_BUILD_TYPE=Release
cmake --build build-web -j
# produces glyph_runtime.html / .js / .wasm; serve via any static server
```
Key flags applied automatically by `cmake/emscripten.cmake`:
`-sUSE_WEBGL2=1 -sFULL_ES3=1 -sALLOW_MEMORY_GROWTH=1 -sASYNCIFY=0 --preload-file assets`

Main loop wrapped via `emscripten_set_main_loop`. Lua hot reload disabled. Touch input enabled.

### Android
- SDL3 ships an `android-project` template. Copy under `platform/android/`.
- `build.gradle` invokes CMake for `arm64-v8a`, `armeabi-v7a`, `x86_64`.
- Asset packaging: assets go in `app/src/main/assets/` and are read via SDL's IO layer (which routes through `AAssetManager`).
- Min API: 24.

### iOS
- CMake with `-GXcode -DCMAKE_SYSTEM_NAME=iOS`.
- Plain Lua only (LuaJIT forbidden by App Store JIT rules).
- Touch input and orientation events through SDL.

---

## 23. Asset pipeline

No bespoke binary format in v1. Ship raw assets:
- PNG (textures, sprite sheets)
- JSON (Aseprite, Tiled)
- OGG (music, long sounds), WAV (short SFX)
- TTF (fonts)
- Lua (scripts)

`asset_root` defaults to `./assets/`. The `Resources::set_root` call adjusts at runtime. On web, `--preload-file assets` bundles them into the wasm package; on Android, they're in the APK; on iOS, in the app bundle's resources.

Optional in v2: a packer tool that produces a single `.glyphpack` file (texture atlases pre-built, JSON preparsed) for ship builds.

---

## 24. Error handling

- Engine internals: exceptions for unrecoverable errors (file not found, GL context failure). Caught by the main loop, logged, app terminates with a popup on desktop / `console.error` on web.
- Lua errors: caught by sol2's `protected_function` wrappers. Logged with stack trace. Script reload puts the engine back into a working state.
- Don't propagate exceptions across the Lua/C++ boundary in either direction beyond what sol2 does for you.

---

## 25. Threading model

- Main thread: events, update, render submission, GL calls.
- Audio thread: owned by miniaudio. Communicate via lock-free command queues (miniaudio handles this internally for the simple API).
- Future: a worker pool for async texture decoding. Out of scope for v1.

All public APIs are main-thread only. Document this explicitly in headers.

---

## 26. Testing

- `tests/` uses doctest (single header).
- Unit tests for: math, Rect intersection, Tiled JSON parsing, Aseprite JSON parsing, sprite batcher overflow behavior, action-binding resolution.
- Sample programs serve as integration tests — they must build and run on every platform per release.

---

## 27. Implementation order (build this in this sequence)

1. **CMake skeleton**, SDL3 submodule, blank window opens and closes cleanly. Sample `01_hello_window`.
2. **GL context + clear color.** Verified on desktop.
3. **Shader + textured quad.** Hardcoded vertices, one texture.
4. **Sprite batcher.** Submit N sprites with M textures, single draw call per flush. Stress test with 10k sprites.
5. **Camera + math.** Mouse pan, scroll-wheel zoom.
6. **Resources + Texture loading** via stb_image. Sample `02_sprite`.
7. **Input system + action mapping.** Sample `03_input` (move a sprite with arrow keys or stick).
8. **Time + fixed timestep.** Verify behavior on slow frame.
9. **Audio** via miniaudio. Sound + music. Volume control.
10. **EnTT integration + Entity façade.** Built-in components.
11. **Built-in systems**: movement, sprite render, animator.
12. **Aseprite JSON loader.** Animated sprite plays.
13. **Tiled TMJ loader + tilemap renderer.** Sample `04_tilemap`.
14. **CollisionSystem** with spatial hash.
15. **Font + text rendering.**
16. **Lua state + sol2 bindings.** Core API exposed.
17. **Script component + ScriptSystem.** Entity scripts work.
18. **Hot reload.**
19. **Web build via Emscripten.** Get `02_sprite` running in a browser.
20. **Android build.** Get `02_sprite` running on a phone.
21. **iOS build.**
22. **Sample 05 — Lua platformer** as the dogfood test.
23. **Documentation pass** on every public header.

---

## 28. Out of scope (v1)

- Networking
- 3D rendering
- Particle system (add post-v1; trivial on top of the batcher)
- Shader hot reload
- Built-in GUI / debug overlay
- Visual editor
- Asset bundling/packing format
- Async loading

These are explicit non-goals — say no to them while building v1, add them in v2.

---

End of spec.
