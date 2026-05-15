cmake_minimum_required(VERSION 3.22)
project(glyph_android LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Resolve repo root from platform/android/app/jni/
get_filename_component(GLYPH_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../../../.." ABSOLUTE)

# Which sample to build. Passed by each Gradle product flavor via cmake.arguments.
if(NOT DEFINED ANDROID_SAMPLE)
    set(ANDROID_SAMPLE "02_sprite")
endif()

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

# Lua 5.4 — stays compiled as C to preserve C linkage of luaL_*/lua_* symbols.
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

target_compile_definitions(glyph PUBLIC GLYPH_PLATFORM_ANDROID)

# android: JNI. log: SDL/__android_log_print. GLESv3+EGL: GLES 3.0 functions.
# OpenSLES: miniaudio's OpenSL ES audio backend.
target_link_libraries(glyph
    PUBLIC  SDL3::SDL3-static glm::glm EnTT::EnTT
    PRIVATE lua54 android log GLESv3 EGL OpenSLES
)

# ---------------------------------------------------------------------------
# Sample shared library — SDLActivity loads this as libmain.so
# ---------------------------------------------------------------------------
add_library(main SHARED
    "${GLYPH_ROOT}/src/main_entry.cpp"
    "${GLYPH_ROOT}/samples/${ANDROID_SAMPLE}/main.cpp"
)

target_include_directories(main PRIVATE
    "${GLYPH_ROOT}/src"
    "${GLYPH_ROOT}/include"
    "${GLYPH_ROOT}/external/stb"
)

target_link_libraries(main PRIVATE glyph)
