# C++ Conventions

Rules that apply to every file in the engine. These exist so the codebase stays consistent as it grows across 23 phases.

---

## Language and compatibility

- **C++17.** No C++20 features — needed for Emscripten, Android NDK, and Xcode toolchains.
- Exceptions **on** — sol2 and EnTT use them.
- RTTI **on** — EnTT and sol2 require it.

---

## Naming

| Kind | Style | Example |
|---|---|---|
| Types (class, struct, enum) | `PascalCase` | `SpriteBatch`, `Entity` |
| Functions and methods | `snake_case` | `draw_sprite`, `screen_to_world` |
| Variables, parameters | `snake_case` | `tile_count`, `cam_pos` |
| Member variables | `snake_case_` (trailing `_`) | `texture_`, `frame_count_` |
| Constants | `kPascalCase` | `kMaxBatchSize` |
| Macros | `SCREAMING_SNAKE` | `GLYPH_MAIN` |
| Namespaces | `lower` | `glyph`, `glyph::detail` |
| Files | `snake_case` | `sprite_batch.cpp` |

No Hungarian notation. No `m_` prefixes.

---

## File layout

- `include/glyph/*.h` — **public API**. What game developers include.
- `src/<subsystem>/*.{h,cpp}` — **implementation and internal headers**. Never `#include`'d from public headers.
- One primary type per file. Filename matches the type.
- `#pragma once` for header guards.

---

## Include order (in `.cpp` files)

1. Corresponding public header (`#include <glyph/renderer.h>`)
2. Other engine internal headers
3. Third-party headers (SDL, glm, etc.)
4. C++ standard library

---

## Memory

- Owning pointers → `std::unique_ptr<T>` or `std::shared_ptr<T>`.
- Non-owning → references (`T&`) or raw pointers (`T*`) when null is meaningful.
- Never `new`/`delete` outside RAII wrappers.
- `Entity` is a value handle — pass by value.

---

## Comments

Comments explain **why**, not what. The code shows what.

```cpp
// SDL_FunctionPointer → void* via memcpy: function and data pointers are the
// same size on all our targets; this is the pedantically-correct conversion.
```

Not:

```cpp
// bind the texture to slot 0
tex.bind(0);   // ← don't do this
```

---

## Platform isolation

`#ifdef GLYPH_PLATFORM_*` guards are only allowed inside `src/platform/`. Everywhere else uses the platform-agnostic API.

---

## Error handling

| Situation | Strategy |
|---|---|
| Engine init failure | `throw std::runtime_error(...)` — caught by main loop, logged, app exits |
| Missing asset | Log + return fallback (magenta texture, empty tilemap) |
| Lua error | Caught by sol2 `protected_function`, logged with stack trace |
| Debug invariant | `GLYPH_ASSERT(condition, message)` — stripped in release |
