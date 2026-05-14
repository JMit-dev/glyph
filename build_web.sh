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

set -e

BUILD_DIR="build_web"

cmake -B "$BUILD_DIR" \
    -DCMAKE_TOOLCHAIN_FILE="$(em-config EMSCRIPTEN_ROOT)/cmake/Modules/Platform/Emscripten.cmake" \
    -DCMAKE_BUILD_TYPE=Release \
    -DGLYPH_BUILD_SAMPLES=ON

cmake --build "$BUILD_DIR" -j"$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)"

echo ""
echo "Build complete. Serve the web output with:"
echo "  python3 -m http.server --directory $BUILD_DIR/web"
echo "Then open: http://localhost:8000/02_sprite/02_sprite.html"
