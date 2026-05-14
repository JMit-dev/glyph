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

## Current phase

**Phase 7: Input system + action mapping** — keyboard, mouse, gamepad; named action bindings; `samples/03_input`.

## Upcoming phases

| Phase | Description | Tag |
|---|---|---|
| 7 | Input system + action mapping · `03_input` | `v0.7.0` |
| 8 | Time + fixed timestep | `v0.8.0` |
| 9 | Audio (miniaudio) | `v0.9.0` |
| 10 | EnTT + Entity façade | `v0.10.0` |
| 11 | Built-in ECS systems | `v0.11.0` |
| 12 | Aseprite JSON loader | `v0.12.0` |
| 13 | Tiled TMJ loader + tilemap renderer · `04_tilemap` | `v0.13.0` |
| 14 | CollisionSystem (spatial hash + AABB) | `v0.14.0` |
| 15 | Font + text rendering | `v0.15.0` |
| 16 | Lua + sol2 bindings | `v0.16.0` |
| 17 | Script component + ScriptSystem | `v0.17.0` |
| 18 | Hot reload | `v0.18.0` |
| 19 | Emscripten web build | `v0.19.0` |
| 20 | Android build | `v0.20.0` |
| 21 | iOS build | `v0.21.0` |
| 22 | Lua platformer sample · `05_lua_platformer` | `v1.0.0` |
| 23 | Documentation pass | `v1.0.1` |
