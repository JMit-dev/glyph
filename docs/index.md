# Glyph Engine

A **code-first, cross-platform 2D game engine**. C++17 core with Lua 5.4 scripting. Write your game once — deploy to Windows, macOS, Linux, Web (WebAssembly), iOS, and Android.

No GUI editor. No drag-and-drop. Your game lives in code.

---

## Design philosophy

- **Code is the editor.** Levels come from [Tiled](https://www.mapeditor.org/) (`.tmj` JSON), sprites from [Aseprite](https://www.aseprite.org/) (`.json` + PNG). Everything else is C++ or Lua.
- **One codebase, every platform.** Platform-specific code is isolated behind interfaces in `src/platform/`. Game code never touches `#ifdef _WIN32`.
- **Boring tech.** SDL3, OpenGL 3.3 / GLES 3.0 / WebGL2, single-header libraries. No bleeding edge.
- **Y-down, pixel units.** Origin at top-left. One world unit = one screen pixel at zoom 1.0. Matches Tiled, SDL, and screen coordinates.

---

## Quick example

```cpp
#include <glyph/glyph.h>

class MyGame : public glyph::Game {
public:
    glyph::AppConfig configure() override {
        glyph::AppConfig c;
        c.title  = "My Game";
        c.width  = 1280;
        c.height = 720;
        return c;
    }

    void on_start() override {
        // load assets, create entities
    }

    void on_update(float dt) override {
        // game logic, input handling
    }

    void on_render(glyph::Renderer& r) override {
        r.clear(glyph::Color::rgba8(30, 30, 30));
        // draw calls
    }
};

GLYPH_MAIN(MyGame)
```

---

## Build

```bash
# Clone with submodules
git clone --recurse-submodules https://github.com/JMit-dev/glyph.git
cd glyph

# Configure and build (requires CMake 3.20+, Python 3, C++17 compiler)
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j

# Run the hello window sample
./build/samples/01_hello_window/01_hello_window
```

See [Getting Started](getting-started.md) for a full walkthrough.

---

## Tech stack

| Concern | Library |
|---|---|
| Window / input | SDL3 3.2+ |
| GL loader (desktop) | glad2 |
| Math | glm 1.0+ *(phase 5)* |
| Image loading | stb_image *(phase 6)* |
| Audio | miniaudio *(phase 9)* |
| ECS | EnTT *(phase 10)* |
| Scripting | Lua 5.4 + sol2 *(phase 16)* |
| JSON | nlohmann/json *(phase 12)* |

All dependencies are vendored as git submodules under `external/`.

---

## Current status

The engine is in active development. See [Build Progress](dev/progress.md) for the current phase.
