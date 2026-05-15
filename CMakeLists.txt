# Glyph 2D game engine
cmake_minimum_required(VERSION 3.20)
project(glyph
    LANGUAGES C CXX
    VERSION 0.1.0
    DESCRIPTION "Code-first 2D game engine"
)

# ---------------------------------------------------------------------------
# Global settings
# ---------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Default to Debug if no build type specified
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

# Put binaries in predictable locations
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# ---------------------------------------------------------------------------
# Platform detection
# ---------------------------------------------------------------------------
if(EMSCRIPTEN)
    set(GLYPH_PLATFORM_WEB ON)
    message(STATUS "Glyph: targeting Web (Emscripten)")
elseif(ANDROID)
    set(GLYPH_PLATFORM_ANDROID ON)
    message(STATUS "Glyph: targeting Android")
elseif(IOS OR CMAKE_SYSTEM_NAME STREQUAL "iOS")
    set(GLYPH_PLATFORM_IOS ON)
    message(STATUS "Glyph: targeting iOS")
else()
    set(GLYPH_PLATFORM_DESKTOP ON)
    message(STATUS "Glyph: targeting Desktop (${CMAKE_SYSTEM_NAME})")
endif()

# ---------------------------------------------------------------------------
# Options
# ---------------------------------------------------------------------------
option(GLYPH_BUILD_SAMPLES "Build sample programs" ON)
option(GLYPH_BUILD_TESTS "Build tests" OFF)

# ---------------------------------------------------------------------------
# Warnings
# ---------------------------------------------------------------------------
if(MSVC)
    add_compile_options(/W4 /permissive-)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# ---------------------------------------------------------------------------
# Dependencies
# ---------------------------------------------------------------------------

# SDL3 — windowing, input, platform abstraction
if(NOT EXISTS "${CMAKE_SOURCE_DIR}/external/SDL/CMakeLists.txt")
    message(FATAL_ERROR
        "SDL3 submodule not found at external/SDL. "
        "Run: git submodule update --init --recursive")
endif()

set(SDL_SHARED OFF CACHE BOOL "" FORCE)
set(SDL_STATIC ON CACHE BOOL "" FORCE)
set(SDL_TEST_LIBRARY OFF CACHE BOOL "" FORCE)
set(SDL_TESTS OFF CACHE BOOL "" FORCE)
set(SDL_INSTALL OFF CACHE BOOL "" FORCE)
add_subdirectory(external/SDL EXCLUDE_FROM_ALL)

# glad2 — OpenGL 3.3 Core function loader (desktop only).
# Sources are pre-generated (external/glad_generated/) so no Python dependency
# is needed at build time. GLES3 is linked directly on mobile/web.
if(GLYPH_PLATFORM_DESKTOP)
    add_library(glad_gl STATIC
        "${CMAKE_SOURCE_DIR}/external/glad_generated/src/gl.c"
    )
    target_include_directories(glad_gl PUBLIC
        "${CMAKE_SOURCE_DIR}/external/glad_generated/include"
    )
endif()

# glm — header-only math library (vec2, mat4, etc.)
if(NOT EXISTS "${CMAKE_SOURCE_DIR}/external/glm/CMakeLists.txt")
    message(FATAL_ERROR
        "glm submodule not found at external/glm. "
        "Run: git submodule update --init --recursive")
endif()
set(GLM_BUILD_TESTS OFF CACHE BOOL "" FORCE)
add_subdirectory(external/glm EXCLUDE_FROM_ALL)

# stb — single-header image loading (stb_image) and font rendering (stb_truetype).
# Implementation translation unit: src/core/stb_impl.cpp
if(NOT EXISTS "${CMAKE_SOURCE_DIR}/external/stb/stb_image.h")
    message(FATAL_ERROR
        "stb submodule not found at external/stb. "
        "Run: git submodule update --init --recursive")
endif()

# EnTT — header-only ECS library.
if(NOT EXISTS "${CMAKE_SOURCE_DIR}/external/entt/single_include/entt/entt.hpp")
    message(FATAL_ERROR
        "EnTT submodule not found at external/entt. "
        "Run: git submodule update --init --recursive")
endif()
set(ENTT_BUILD_TESTING OFF CACHE BOOL "" FORCE)
set(ENTT_INSTALL       OFF CACHE BOOL "" FORCE)
add_subdirectory(external/entt EXCLUDE_FROM_ALL)

# miniaudio — single-header audio library.
# Implementation translation unit: src/audio/miniaudio_impl.cpp
if(NOT EXISTS "${CMAKE_SOURCE_DIR}/external/miniaudio/miniaudio.h")
    message(FATAL_ERROR
        "miniaudio submodule not found at external/miniaudio. "
        "Run: git submodule update --init --recursive")
endif()

# nlohmann/json — header-only JSON parser (Aseprite + Tiled loaders).
if(NOT EXISTS "${CMAKE_SOURCE_DIR}/external/json/single_include/nlohmann/json.hpp")
    message(FATAL_ERROR
        "nlohmann/json submodule not found at external/json. "
        "Run: git submodule update --init --recursive")
endif()

# Lua 5.4 — compiled from source; lua.c (interpreter) and luac.c (compiler)
# are excluded so we build only the library.
if(NOT EXISTS "${CMAKE_SOURCE_DIR}/external/lua/lua.h")
    message(FATAL_ERROR
        "Lua submodule not found at external/lua. "
        "Run: git submodule update --init --recursive")
endif()
file(GLOB _LUA_SOURCES "${CMAKE_SOURCE_DIR}/external/lua/*.c")
list(FILTER _LUA_SOURCES EXCLUDE REGEX "[/\\\\](lua|luac|onelua)\\.c$")
add_library(lua54 STATIC ${_LUA_SOURCES})
target_include_directories(lua54 PUBLIC "${CMAKE_SOURCE_DIR}/external/lua")
if(MSVC)
    target_compile_definitions(lua54 PRIVATE _CRT_SECURE_NO_WARNINGS)
    target_compile_options(lua54 PRIVATE /wd4324)
endif()
if(EMSCRIPTEN)
    # Lua stays compiled as C (preserves C linkage of luaL_* / lua_* symbols).
    # Emscripten emulates setjmp/longjmp via -enable-emscripten-sjlj so Lua's
    # default error path works correctly. No LANGUAGE CXX override needed.
    # sol2 always wraps Lua calls with lua_pcall, so errors never longjmp
    # across C++ destructors in our code.
endif()

# sol2 — header-only C++ Lua binding layer.
if(NOT EXISTS "${CMAKE_SOURCE_DIR}/external/sol2/include/sol/sol.hpp")
    message(FATAL_ERROR
        "sol2 submodule not found at external/sol2. "
        "Run: git submodule update --init --recursive")
endif()

# ---------------------------------------------------------------------------
# Engine library
# ---------------------------------------------------------------------------
add_library(glyph STATIC
    src/platform/window.cpp
    src/platform/file_io.cpp
    src/core/app.cpp
    src/core/time.cpp
    src/core/stb_impl.cpp
    src/core/resources.cpp
    src/core/font.cpp
    src/input/input.cpp
    src/audio/miniaudio_impl.cpp
    src/audio/audio.cpp
    src/scene/scene.cpp
    src/scene/systems.cpp
    src/scene/sprite_sheet.cpp
    src/scene/tilemap.cpp
    src/lua/lua_state.cpp
    src/lua/lua_bindings.cpp
    src/lua/script_system.cpp
    src/lua/lua_hotreload.cpp
    src/renderer/renderer.cpp
    src/renderer/shader.cpp
    src/renderer/texture.cpp
    src/renderer/sprite_batch.cpp
    src/renderer/camera.cpp
)

target_include_directories(glyph
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/external/stb
        ${CMAKE_SOURCE_DIR}/external/miniaudio
        ${CMAKE_SOURCE_DIR}/external/json/single_include
        ${CMAKE_SOURCE_DIR}/external/sol2/include
)

target_compile_definitions(glyph PUBLIC
    $<$<BOOL:${GLYPH_PLATFORM_DESKTOP}>:GLYPH_PLATFORM_DESKTOP>
    $<$<BOOL:${GLYPH_PLATFORM_WEB}>:GLYPH_PLATFORM_WEB>
    $<$<BOOL:${GLYPH_PLATFORM_ANDROID}>:GLYPH_PLATFORM_ANDROID>
    $<$<BOOL:${GLYPH_PLATFORM_IOS}>:GLYPH_PLATFORM_IOS>
)

target_link_libraries(glyph
    PUBLIC
        SDL3::SDL3-static
        glm::glm
        EnTT::EnTT
        $<$<BOOL:${GLYPH_PLATFORM_DESKTOP}>:glad_gl>
    PRIVATE
        lua54
)

# miniaudio platform audio backends.
# Windows links via #pragma comment(lib,...) in the IMPLEMENTATION block.
# Linux needs dl, pthread, m. macOS needs CoreAudio/CoreFoundation.
# Emscripten uses the Web Audio API via miniaudio's webaudio backend — no extra libs.
if(UNIX AND NOT APPLE AND NOT EMSCRIPTEN AND NOT ANDROID)
    target_link_libraries(glyph PUBLIC dl pthread m)
elseif(APPLE)
    target_link_libraries(glyph PUBLIC
        "-framework CoreAudio"
        "-framework CoreFoundation"
        "-framework AudioToolbox")
endif()

# Emscripten: C++ exceptions must be enabled so Lua (compiled as C++) and
# sol2 can propagate errors correctly across WASM frames.
if(EMSCRIPTEN)
    target_compile_options(glyph PRIVATE -fexceptions)
endif()

# ---------------------------------------------------------------------------
# Runtime entry point (OBJECT lib so main() always links into the executable)
# ---------------------------------------------------------------------------
add_library(glyph_runtime OBJECT src/main_entry.cpp)

target_include_directories(glyph_runtime
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(glyph_runtime PRIVATE SDL3::SDL3-static glm::glm EnTT::EnTT)

# ---------------------------------------------------------------------------
# Helper: glyph_add_executable(<target> <sources...>)
# Use this in samples and games. It links the engine + runtime entry.
# ---------------------------------------------------------------------------
function(glyph_add_executable target)
    add_executable(${target} ${ARGN})
    target_link_libraries(${target} PRIVATE glyph glyph_runtime)

    if(EMSCRIPTEN)
        # Output an HTML page with embedded JS loader; WASM lives alongside.
        set_target_properties(${target} PROPERTIES SUFFIX ".html")
        set_target_properties(${target} PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/web/${target})

        target_compile_options(${target} PRIVATE -fexceptions)
        target_link_options(${target} PRIVATE
            # WebGL2 / GLES 3.0
            "-sUSE_WEBGL2=1"
            "-sFULL_ES3=1"
            "-sMIN_WEBGL_VERSION=2"
            "-sMAX_WEBGL_VERSION=2"
            # Memory
            "-sALLOW_MEMORY_GROWTH=1"
            "-sSTACK_SIZE=2097152"      # 2 MB — Lua needs more stack than the default
            "-sINITIAL_MEMORY=67108864" # 64 MB initial heap
            "-sNO_EXIT_RUNTIME=1"
            # Virtual filesystem — needed for asset and script file access
            "-sFORCE_FILESYSTEM=1"
            # C++ exceptions required for Lua/sol2
            "-fexceptions"
            # Custom HTML shell — replaces the default Emscripten page
            "--shell-file" "${CMAKE_SOURCE_DIR}/cmake/web/shell.html"
        )
    else()
        set_target_properties(${target} PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/samples/${target})
    endif()
endfunction()

# ---------------------------------------------------------------------------
# Samples
# ---------------------------------------------------------------------------
if(GLYPH_BUILD_SAMPLES)
    add_subdirectory(samples/01_hello_window)
    add_subdirectory(samples/02_sprite)
    add_subdirectory(samples/03_input)
    add_subdirectory(samples/04_tilemap)
endif()
