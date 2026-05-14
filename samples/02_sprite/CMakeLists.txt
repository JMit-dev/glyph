glyph_add_executable(02_sprite main.cpp)

# stb_image_write — only for generating the test PNG; not part of the engine.
target_include_directories(02_sprite PRIVATE ${CMAKE_SOURCE_DIR}/external/stb)
# Suppress MSVC's 'sprintf is unsafe' warning from stb_image_write internals.
if(MSVC)
    target_compile_definitions(02_sprite PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()
