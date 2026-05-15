cmake_minimum_required(VERSION 3.22)
project(glyph_ios LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Must be invoked with -DCMAKE_SYSTEM_NAME=iOS -GXcode
if(NOT CMAKE_SYSTEM_NAME STREQUAL "iOS")
    message(FATAL_ERROR
        "This CMakeLists.txt is for iOS only.\n"
        "Configure with: cmake -GXcode -DCMAKE_SYSTEM_NAME=iOS ...")
endif()

get_filename_component(GLYPH_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../.." ABSOLUTE)

set(CMAKE_OSX_DEPLOYMENT_TARGET "15.0" CACHE STRING "")
# Build for device + simulator so both work from one archive
set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "")

# ---------------------------------------------------------------------------
# Dependencies
# ---------------------------------------------------------------------------
set(SDL_SHARED OFF CACHE BOOL "" FORCE)
set(SDL_STATIC ON CACHE BOOL "" FORCE)
set(SDL_TEST_LIBRARY OFF CACHE BOOL "" FORCE)
set(SDL_TESTS OFF CACHE BOOL "" FORCE)
set(SDL_INSTALL OFF CACHE BOOL "" FORCE)
add_subdirectory("${GLYPH_ROOT}/external/SDL" SDL EXCLUDE_FROM_ALL)

set(GLM_BUILD_TESTS OFF CACHE BOOL "" FORCE)
add_subdirectory("${GLYPH_ROOT}/external/glm" glm EXCLUDE_FROM_ALL)

set(ENTT_BUILD_TESTING OFF CACHE BOOL "" FORCE)
set(ENTT_INSTALL       OFF CACHE BOOL "" FORCE)
add_subdirectory("${GLYPH_ROOT}/external/entt" entt EXCLUDE_FROM_ALL)

# Lua 5.4 — plain C (LuaJIT is banned from App Store due to JIT memory mapping)
file(GLOB _LUA_SOURCES "${GLYPH_ROOT}/external/lua/*.c")
list(FILTER _LUA_SOURCES EXCLUDE REGEX "[/\\\\](lua|luac|onelua)\\.c$")
add_library(lua54 STATIC ${_LUA_SOURCES})
target_include_directories(lua54 PUBLIC "${GLYPH_ROOT}/external/lua")

# ---------------------------------------------------------------------------
# Engine static library
# ---------------------------------------------------------------------------
add_library(glyph STATIC
    "${GLYPH_ROOT}/src/platform/window.cpp"
    "${GLYPH_ROOT}/src/platform/file_io.cpp"
    "${GLYPH_ROOT}/src/core/app.cpp"
    "${GLYPH_ROOT}/src/core/time.cpp"
    "${GLYPH_ROOT}/src/core/stb_impl.cpp"
    "${GLYPH_ROOT}/src/core/resources.cpp"
    "${GLYPH_ROOT}/src/core/font.cpp"
    "${GLYPH_ROOT}/src/input/input.cpp"
    "${GLYPH_ROOT}/src/audio/miniaudio_impl.cpp"
    "${GLYPH_ROOT}/src/audio/audio.cpp"
    "${GLYPH_ROOT}/src/scene/scene.cpp"
    "${GLYPH_ROOT}/src/scene/systems.cpp"
    "${GLYPH_ROOT}/src/scene/sprite_sheet.cpp"
    "${GLYPH_ROOT}/src/scene/tilemap.cpp"
    "${GLYPH_ROOT}/src/lua/lua_state.cpp"
    "${GLYPH_ROOT}/src/lua/lua_bindings.cpp"
    "${GLYPH_ROOT}/src/lua/script_system.cpp"
    "${GLYPH_ROOT}/src/lua/lua_hotreload.cpp"
    "${GLYPH_ROOT}/src/renderer/renderer.cpp"
    "${GLYPH_ROOT}/src/renderer/shader.cpp"
    "${GLYPH_ROOT}/src/renderer/texture.cpp"
    "${GLYPH_ROOT}/src/renderer/sprite_batch.cpp"
    "${GLYPH_ROOT}/src/renderer/camera.cpp"
)

target_include_directories(glyph
    PUBLIC  "${GLYPH_ROOT}/include"
    PRIVATE "${GLYPH_ROOT}/src"
            "${GLYPH_ROOT}/external/stb"
            "${GLYPH_ROOT}/external/miniaudio"
            "${GLYPH_ROOT}/external/json/single_include"
            "${GLYPH_ROOT}/external/sol2/include"
)

target_compile_definitions(glyph PUBLIC GLYPH_PLATFORM_IOS)

# iOS system frameworks required by SDL3, miniaudio, and OpenGLES.
target_link_libraries(glyph
    PUBLIC  SDL3::SDL3-static glm::glm EnTT::EnTT
    PRIVATE lua54
            "-framework UIKit"
            "-framework OpenGLES"
            "-framework CoreGraphics"
            "-framework CoreFoundation"
            "-framework Foundation"
            "-framework AudioToolbox"
            "-framework CoreAudio"
            "-framework AVFoundation"
            "-framework GameController"
            "-framework CoreMotion"
            "-framework CoreBluetooth"
            "-framework CoreHaptics"
)

# ---------------------------------------------------------------------------
# Helper: one iOS .app per sample
# ---------------------------------------------------------------------------
function(glyph_ios_app target sample_dir label)
    set(PLIST_IN "${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.in")

    add_executable(${target} MACOSX_BUNDLE
        "${GLYPH_ROOT}/src/main_entry.cpp"
        "${GLYPH_ROOT}/samples/${sample_dir}/main.cpp"
        "${PLIST_IN}"          # listed so Xcode shows it in the project
    )

    target_include_directories(${target} PRIVATE
        "${GLYPH_ROOT}/src"
        "${GLYPH_ROOT}/include"
        "${GLYPH_ROOT}/external/stb"
    )

    target_link_libraries(${target} PRIVATE glyph)

    # Configure the Info.plist for this specific target
    set(GLYPH_APP_BUNDLE_ID   "dev.glyph.samples.${target}")
    set(GLYPH_APP_DISPLAY_NAME "${label}")
    configure_file("${PLIST_IN}"
        "${CMAKE_CURRENT_BINARY_DIR}/${target}_Info.plist")

    set_target_properties(${target} PROPERTIES
        MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_BINARY_DIR}/${target}_Info.plist"
        XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "dev.glyph.samples.${target}"
        XCODE_ATTRIBUTE_TARGETED_DEVICE_FAMILY    "1,2"  # iPhone + iPad
        XCODE_ATTRIBUTE_IPHONEOS_DEPLOYMENT_TARGET "15.0"
        XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY         "iPhone Developer"
        XCODE_ATTRIBUTE_ENABLE_BITCODE             "NO"
        XCODE_ATTRIBUTE_SUPPORTS_MACCATALYST       "NO"
    )
endfunction()

# ---------------------------------------------------------------------------
# Sample targets
# ---------------------------------------------------------------------------
glyph_ios_app(01_hello_window 01_hello_window "Glyph: Hello Window")
glyph_ios_app(02_sprite       02_sprite       "Glyph: Sprite")
glyph_ios_app(03_input        03_input        "Glyph: Input")
glyph_ios_app(04_tilemap      04_tilemap      "Glyph: Tilemap")
