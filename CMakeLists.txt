# Glyph 2D game engine
cmake_minimum_required(VERSION 3.20)
project(glyph
    LANGUAGES CXX
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
# Requires Python at configure time to generate the loader source.
# GLES3 is linked directly on mobile/web; no loader needed there.
if(GLYPH_PLATFORM_DESKTOP)
    if(NOT EXISTS "${CMAKE_SOURCE_DIR}/external/glad/cmake/CMakeLists.txt")
        message(FATAL_ERROR
            "glad submodule not found at external/glad. "
            "Run: git submodule update --init --recursive")
    endif()
    add_subdirectory(external/glad/cmake EXCLUDE_FROM_ALL)
    glad_add_library(glad_gl STATIC REPRODUCIBLE API gl:core=3.3)
endif()

# glm — header-only math library (vec2, mat4, etc.)
if(NOT EXISTS "${CMAKE_SOURCE_DIR}/external/glm/CMakeLists.txt")
    message(FATAL_ERROR
        "glm submodule not found at external/glm. "
        "Run: git submodule update --init --recursive")
endif()
set(GLM_BUILD_TESTS OFF CACHE BOOL "" FORCE)
add_subdirectory(external/glm EXCLUDE_FROM_ALL)

# ---------------------------------------------------------------------------
# Engine library
# ---------------------------------------------------------------------------
add_library(glyph STATIC
    src/platform/window.cpp
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
        $<$<BOOL:${GLYPH_PLATFORM_DESKTOP}>:glad_gl>
)

# ---------------------------------------------------------------------------
# Runtime entry point (OBJECT lib so main() always links into the executable)
# ---------------------------------------------------------------------------
add_library(glyph_runtime OBJECT src/main_entry.cpp)

target_include_directories(glyph_runtime
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(glyph_runtime PRIVATE SDL3::SDL3-static glm::glm)

# ---------------------------------------------------------------------------
# Helper: glyph_add_executable(<target> <sources...>)
# Use this in samples and games. It links the engine + runtime entry.
# ---------------------------------------------------------------------------
function(glyph_add_executable target)
    add_executable(${target} ${ARGN})
    target_link_libraries(${target}
        PRIVATE
            glyph
            glyph_runtime
    )
    set_target_properties(${target} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/samples/${target}
    )
endfunction()

# ---------------------------------------------------------------------------
# Samples
# ---------------------------------------------------------------------------
if(GLYPH_BUILD_SAMPLES)
    add_subdirectory(samples/01_hello_window)
endif()
