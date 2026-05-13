# Code conventions

These rules apply to every C++ file in the engine. Follow them on every edit, including small ones.

## Language

- **C++17.** No C++20 features even if your compiler supports them — we need broad toolchain compat across desktop, Emscripten, Android NDK, Xcode.
- Exceptions are allowed (sol2 and EnTT use them). Don't throw from hot loops.
- RTTI is allowed (EnTT and sol2 require it).

## File organization

- Public headers: `include/glyph/*.h`. Anything a game developer needs to include lives here.
- Implementation and internal headers: `src/<subsystem>/*.{h,cpp}`. Never `#include` these from public headers.
- One primary type per file. Filename matches the type in `snake_case` (`sprite_batch.cpp` defines `SpriteBatch`).
- Header guards: `#pragma once`. No `#ifndef GLYPH_FOO_H` style.
- Headers include only what they need. Forward-declare in headers where possible; full includes in `.cpp`.

## Naming

| Kind | Style | Example |
|---|---|---|
| Types (class, struct, enum) | `PascalCase` | `SpriteBatch`, `Entity` |
| Functions and methods | `snake_case` | `draw_sprite`, `screen_to_world` |
| Variables, parameters | `snake_case` | `tile_count`, `cam_pos` |
| Member variables | `snake_case_` (trailing underscore) | `texture_`, `frame_count_` |
| Constants | `kPascalCase` or `SCREAMING_SNAKE` for macros | `kMaxBatchSize`, `GLYPH_MAIN` |
| Namespaces | `lower` | `glyph`, `glyph::detail` |
| Files | `snake_case` | `sprite_batch.cpp` |

Avoid Hungarian notation. Avoid `m_` prefixes (we use trailing `_`).

## Memory and ownership

- Owning pointers → `std::unique_ptr<T>` or `std::shared_ptr<T>`. Resources cached by `Resources` are `shared_ptr`.
- Non-owning references → references (`T&`) where possible, raw pointers (`T*`) only when null is meaningful.
- Never `new`/`delete` outside RAII wrappers.
- `Entity` is a value-typed handle, not a pointer. Pass by value.
- Containers: `std::vector` by default. `std::unordered_map` for keyed lookup. Don't reach for `std::map` (ordered) without a reason.

## Includes

Order in `.cpp` files:
1. Corresponding header (`#include "sprite_batch.h"`)
2. Other engine internal headers
3. Public engine headers (`#include <glyph/...>`)
4. Third-party headers (SDL, glm, etc.)
5. C++ standard library

One include per line. No `using namespace` at file scope in headers. `using namespace glyph;` is allowed in `.cpp` files when it improves readability.

## Comments

- Comments explain **why**, not **what**. The code shows what.
- Public headers get brief docstrings on each function (1–3 lines). No doxygen — plain comments.
- TODO/FIXME tags: `// TODO(name): ...` with a sentence describing what's missing or wrong. Don't leave bare `TODO`s.

## Error handling

- Engine init failures: throw `std::runtime_error` with a useful message. Caught by main loop, logged, app exits.
- Recoverable errors (missing texture, malformed JSON): log and return a fallback (magenta texture, empty tilemap). Don't crash.
- Lua errors: caught by sol2 `protected_function`. Logged with stack trace.
- Asserts: `GLYPH_ASSERT(condition, message)` macro — fires in debug builds, compiled out in release. Use for invariants that should never fail in correct code.

## Performance

- The renderer hot path (`SpriteBatch::submit`) is the one place to micro-optimize. Avoid allocations per submit. Avoid virtual calls per submit.
- Everywhere else: clarity first. Profile before optimizing.
- `std::string` and `std::vector` are fine for setup/teardown. Avoid them in per-frame inner loops.

## Don't

- Don't add a new third-party dependency without updating spec §2 and asking first.
- Don't introduce platform `#ifdef`s outside `src/platform/`.
- Don't add singletons. The `Game` instance is the root; everything hangs off it.
- Don't write your own math types when glm has them.
- Don't write your own threading primitives. We're single-threaded by design (see spec §25).
