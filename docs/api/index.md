# API Reference

All public headers live under `include/glyph/`. Include the umbrella header to get everything:

```cpp
#include <glyph/glyph.h>
```

Or include individual headers:

```cpp
#include <glyph/app.h>       // Game, AppConfig, GLYPH_MAIN
#include <glyph/renderer.h>  // Renderer
#include <glyph/texture.h>   // Texture
#include <glyph/math.h>      // Color, Rect, ivec2
```

---

## Available modules

| Header | Contents | Since |
|---|---|---|
| [`app.h`](game.md) | `Game`, `AppConfig`, `GLYPH_MAIN` | v0.1.0 |
| [`renderer.h`](renderer.md) | `Renderer` | v0.2.0 |
| [`texture.h`](texture.md) | `Texture` | v0.3.0 |
| [`math.h`](math.md) | `Color`, `Rect`, `ivec2` | v0.2.0 |

---

## Coming in future phases

| Header | Contents | Phase |
|---|---|---|
| `math.h` (extended) | `vec2`, `vec3`, `mat4` via glm | 5 |
| `resources.h` | `Resources` — ref-counted asset cache | 6 |
| `input.h` | `Input`, action mappings | 7 |
| `time.h` | `Time`, fixed timestep | 8 |
| `audio.h` | `Audio`, `Sound`, `Music` | 9 |
| `entity.h` | `Entity` handle | 10 |
| `scene.h` | `Scene`, ECS systems | 10 |
| `components.h` | `Transform`, `Sprite`, `Animator`, etc. | 10 |
| `sprite_sheet.h` | Aseprite JSON loader | 12 |
| `tilemap.h` | Tiled TMJ loader | 13 |
| `lua_api.h` | Lua scripting layer | 16 |
