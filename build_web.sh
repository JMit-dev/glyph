#!/usr/bin/env bash
# build_web.sh — build all Glyph samples for the web using Emscripten.
#
# Prerequisites:
#   1. Install emsdk:  https://emscripten.org/docs/getting_started/downloads.html
#      git clone https://github.com/emscripten-core/emsdk.git
#      cd emsdk && ./emsdk install latest && ./emsdk activate latest
#   2. Source the environment (run this script from the repo root):
#      source /path/to/emsdk/emsdk_env.sh
#
# Usage:
#   chmod +x build_web.sh
#   source /path/to/emsdk/emsdk_env.sh
#   ./build_web.sh
#
# Output lands in build_web/web/<sample>/<sample>.html
# Serve with:  python3 -m http.server --directory build_web/web
#
# Notes:
#   - Requires Ninja (https://ninja-build.org/). MinGW Makefiles do not work
#     with Emscripten on Windows due to .bat compiler wrapper limitations.
#   - On Linux/macOS, Ninja or Unix Makefiles both work; Ninja is preferred.

set -e

BUILD_DIR="build_web"

# Detect generator: prefer Ninja, fall back to Unix Makefiles on non-Windows.
if command -v ninja &>/dev/null; then
    GENERATOR="Ninja"
elif [ "$OSTYPE" != "msys" ] && [ "$OSTYPE" != "cygwin" ]; then
    GENERATOR="Unix Makefiles"
else
    echo "Error: Ninja is required on Windows. Install from https://ninja-build.org/"
    exit 1
fi

cmake -B "$BUILD_DIR" \
    -DCMAKE_TOOLCHAIN_FILE="$(em-config EMSCRIPTEN_ROOT)/cmake/Modules/Platform/Emscripten.cmake" \
    -DCMAKE_BUILD_TYPE=Release \
    -DGLYPH_BUILD_SAMPLES=ON \
    -G "$GENERATOR"

cmake --build "$BUILD_DIR" -j"$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)"

echo ""
echo "Build complete. Serve the web output with:"
echo "  python3 -m http.server --directory $BUILD_DIR/web"
echo "Then open: http://localhost:8000/02_sprite/02_sprite.html"
