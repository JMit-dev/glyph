# Build Progress

Tracks position in the 23-step implementation order from the engine spec.

## Completed phases

| Phase | Description | Tag |
|---|---|---|
| ✅ 1 | CMake skeleton + SDL3 window — `01_hello_window` opens cleanly | `v0.1.0` |
| ✅ 2 | GL context + clear color — glad2 loaded, vsync swap | `v0.2.0` |
| ✅ 3 | Shader + textured quad — sprite shader, `draw_textured_quad` | `v0.3.0` |
| ✅ 4 | Sprite batcher — 2000-quad buffer, 8-texture slots, auto-flush | `v0.4.0` |
| ✅ 5 | Camera + math — glm aliases, `Camera::view_projection`, pan/zoom | `v0.5.0` |
| ✅ 6 | Resources + Texture loading — stb_image, `Resources` cache, `02_sprite` | `v0.6.0` |
| ✅ 7 | Input system + action mapping — `Key`/`MouseButton`, axes, `03_input` | `v0.7.0` |
| ✅ 8 | Time + fixed timestep — `on_fixed_update`, `fixed_alpha`, smoothed FPS | `v0.8.0` |
| ✅ 9 | Audio — miniaudio, `Sound`/`Music`, `SoundHandle`, volume, fade | `v0.9.0` |
| ✅ 10 | EnTT + Entity façade — `Scene`, `Entity`, `components.h` | `v0.10.0` |
| ✅ 11 | Built-in systems — `MovementSystem`, `LifetimeSystem`, `Scene::render()` with sprite sorting | `v0.11.0` |
| ✅ 12 | Aseprite JSON loader — `SpriteSheet::load()`, `AnimClip`, `AnimatorSystem` | `v0.12.0` |
| ✅ 13 | Tiled TMJ loader + tilemap renderer — `TilemapRef`, `samples/04_tilemap` | `v0.13.0` |
| ✅ 14 | CollisionSystem — spatial hash broad-phase, AABB narrow-phase, `on_collision` callback | `v0.14.0` |
| ✅ 15 | Font + text rendering — `Font::load()`, stb_truetype atlas, `Renderer::draw_text()` | `v0.15.0` |
| ✅ 16 | Lua 5.4 + sol2 bindings — `LuaState`, core API, `glyph.*` table in Lua | `v0.16.0` |
| ✅ 17 | Script component + ScriptSystem — `on_start`, `on_update`, `on_collision` per entity | `v0.17.0` |
| ✅ 18 | Hot reload — mtime polling, metatable patching, global script re-execution (desktop only) | `v0.18.0` |
| ✅ 19 | Emscripten web build — WebGL2, Web Audio, Lua/WASM; all modules on all platforms | `v0.19.0` |
| ✅ 20 | Android build — all 4 samples as APKs, Gradle product flavors, NDK CMake | `v0.20.0` |
| ✅ 21 | iOS build — all 4 samples as .app bundles, CMake Xcode generator | `v0.21.0` |

## Current phase

**Phase 22: Lua platformer sample**

## Upcoming phases

| Phase | Description | Tag |
|---|---|---|
| 22 | Lua platformer sample · `05_lua_platformer` | `v1.0.0` |
| 23 | Documentation pass | `v1.0.1` |
| 22 | Lua platformer sample · `05_lua_platformer` | `v1.0.0` |
| 23 | Documentation pass | `v1.0.1` |
