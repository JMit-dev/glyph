# Glyph

A code-first 2D game engine. C++17 core, Lua scripting, cross-platform.

See `glyph_engine_spec.md` for the architecture spec and `.claude/` for project rules.

## Setup

Dependencies are vendored as git submodules.

### Fresh clone

```bash
git clone <your-repo-url> glyph
cd glyph
git submodule update --init --recursive
```

### Adding SDL3 submodule (first time only)

If you're setting up the repo for the first time:

```bash
git submodule add https://github.com/libsdl-org/SDL.git external/SDL
cd external/SDL
git checkout release-3.2.0   # or the latest stable 3.x tag
cd ../..
git add .gitmodules external/SDL
git commit -m "build: add SDL3 submodule at release-3.2.0"
```

## Build (desktop)

Requires CMake 3.20+, a C++17 compiler.

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j
```

Run the hello-window sample:

```bash
./build/samples/01_hello_window/01_hello_window
```

A blank window opens. Close it to exit. Phase 1 complete.

## Project layout

| Path | Contents |
|---|---|
| `include/glyph/` | Public API headers |
| `src/` | Engine implementation |
| `samples/` | Sample programs (one per phase milestone) |
| `external/` | Vendored dependencies (git submodules) |
| `glyph_engine_spec.md` | Architecture spec — source of truth |
| `.claude/` | AI assistant config and progress tracking |
| `CLAUDE.md` | Entry point for AI sessions |
