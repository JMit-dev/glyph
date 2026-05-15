# Getting Started

## Prerequisites

| Tool | Version | Notes |
|---|---|---|
| CMake | 3.20+ | Build system |
| C++ compiler | C++17 | MSVC 2019+, GCC 9+, Clang 10+ |
| Python | 3.8+ | Required by glad2 at CMake configure time |
| Git | any | For submodule management |

---

## Cloning

Always clone with `--recurse-submodules` to pull SDL3, glad2, and future dependencies:

```bash
git clone --recurse-submodules https://github.com/JMit-dev/glyph.git
cd glyph
```

If you already cloned without the flag:

```bash
git submodule update --init --recursive
```

---

## Building (Desktop)

```bash
# Debug build
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j

# Release build
cmake -B build-release -DCMAKE_BUILD_TYPE=Release
cmake --build build-release -j
```

Binaries land in `build/samples/<name>/`.

---

## Building (Web — Emscripten)

**Prerequisites:**

1. Install [emsdk](https://emscripten.org/docs/getting_started/downloads.html):
   ```bash
   git clone https://github.com/emscripten-core/emsdk.git
   cd emsdk && ./emsdk install latest && ./emsdk activate latest
   source emsdk_env.sh   # adds emcc to PATH
   ```
2. Install [Ninja](https://ninja-build.org/) — required on Windows; recommended on all platforms.
   On Windows: download `ninja-win.zip` from the Ninja releases page and add it to your PATH.

Then build from the repo root:

```bash
source /path/to/emsdk/emsdk_env.sh
./build_web.sh        # wraps cmake with the Emscripten toolchain + Ninja
```

Or manually:

```bash
cmake -B build_web \
    -DCMAKE_TOOLCHAIN_FILE="$(em-config EMSCRIPTEN_ROOT)/cmake/Modules/Platform/Emscripten.cmake" \
    -DCMAKE_BUILD_TYPE=Release \
    -G Ninja
cmake --build build_web -j
```

HTML/JS/WASM output lands in `build_web/web/<sample>/`. Serve locally (browsers block local file:// WebGL):

```bash
python3 -m http.server --directory build_web/web
# open http://localhost:8000/02_sprite/02_sprite.html
```

**All engine modules work on web:** rendering (WebGL2), audio (Web Audio via miniaudio), input (SDL3), ECS, Lua scripting. Hot reload is disabled on web (no filesystem watcher) — scripts are baked into the WASM bundle at build time.

---

## Running the samples

```bash
# Hello window — verifies GL context and textured quad
./build/samples/01_hello_window/01_hello_window      # Linux/macOS
build\samples\01_hello_window\Debug\01_hello_window.exe  # Windows
```

You should see a cornflower-blue window with a black-and-white checkerboard in the center.

---

## Writing your first game

**1.** Create a directory under `samples/`:

```
samples/my_game/
├── CMakeLists.txt
└── main.cpp
```

**2.** `CMakeLists.txt`:

```cmake
glyph_add_executable(my_game main.cpp)
```

**3.** Register it in the root `CMakeLists.txt`:

```cmake
if(GLYPH_BUILD_SAMPLES)
    add_subdirectory(samples/01_hello_window)
    add_subdirectory(samples/my_game)   # add this
endif()
```

**4.** `main.cpp`:

```cpp
#include <glyph/glyph.h>

class MyGame : public glyph::Game {
public:
    glyph::AppConfig configure() override {
        glyph::AppConfig c;
        c.title = "My Game";
        return c;
    }

    void on_render(glyph::Renderer& r) override {
        r.clear(glyph::Color::rgba8(40, 40, 40));
    }
};

GLYPH_MAIN(MyGame)
```

**5.** Rebuild:

```bash
cmake --build build -j
```

---

## CMake options

| Option | Default | Description |
|---|---|---|
| `GLYPH_BUILD_SAMPLES` | `ON` | Build the samples directory |
| `GLYPH_BUILD_TESTS` | `OFF` | Build the test suite |
| `CMAKE_BUILD_TYPE` | `Debug` | `Debug`, `Release`, `RelWithDebInfo` |

---

## Platform compile definitions

These are automatically set by CMake and available in all engine and game source files:

| Macro | Platform |
|---|---|
| `GLYPH_PLATFORM_DESKTOP` | Windows, macOS, Linux |
| `GLYPH_PLATFORM_WEB` | Emscripten / WebAssembly |
| `GLYPH_PLATFORM_ANDROID` | Android NDK |
| `GLYPH_PLATFORM_IOS` | iOS |
