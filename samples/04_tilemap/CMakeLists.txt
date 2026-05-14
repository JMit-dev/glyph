glyph_add_executable(04_tilemap main.cpp)

# stb_image_write — only for generating the test tileset PNG; not part of the engine.
target_include_directories(04_tilemap PRIVATE ${CMAKE_SOURCE_DIR}/external/stb)
if(MSVC)
    target_compile_definitions(04_tilemap PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()
