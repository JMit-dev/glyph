#!/usr/bin/env bash
# build_ios.sh — build all Glyph samples for iOS using Xcode.
#
# Prerequisites:
#   - Xcode 14+ with iOS 15+ SDK
#   - Valid Apple Developer account for device builds (simulator works without one)
#
# Usage:
#   ./platform/ios/build_ios.sh [--device|--simulator]
#
# Output: platform/ios/build_ios/
# Open the generated .xcodeproj in Xcode to run on device.

set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"
BUILD_DIR="$SCRIPT_DIR/build_ios"
TARGET="${1:---simulator}"

cmake -S "$SCRIPT_DIR" -B "$BUILD_DIR" \
    -G Xcode \
    -DCMAKE_SYSTEM_NAME=iOS \
    -DCMAKE_OSX_DEPLOYMENT_TARGET=15.0 \
    -DCMAKE_BUILD_TYPE=Release

if [ "$TARGET" = "--device" ]; then
    SDK="iphoneos"
else
    SDK="iphonesimulator"
fi

cmake --build "$BUILD_DIR" \
    --config Release \
    -- \
    -sdk "$SDK" \
    -jobs "$(sysctl -n hw.ncpu)"

echo ""
echo "Build complete. Open $BUILD_DIR/glyph_ios.xcodeproj in Xcode to run on device."
