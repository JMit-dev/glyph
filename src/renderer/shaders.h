// shaders.h — embedded GLSL source strings.
//
// Shaders carry no #version directive; the platform header is prepended at
// compile time via the two-source-string glShaderSource trick:
//   Desktop GL 3.3:  "#version 330 core\n"
//   GLES 3.0 / Web:  "#version 300 es\n"
//
// #ifdef GL_ES guards precision qualifiers so the same source compiles on
// both profiles without modification.
//
// The fragment shader uses a switch on tex_index rather than a dynamic array
// lookup — some GLES 3.0 drivers reject non-constant sampler array indices.
#pragma once

namespace glyph::shaders {

inline constexpr const char* kSpriteVert = R"GLSL(
layout(location = 0) in vec2  a_pos;
layout(location = 1) in vec2  a_uv;
layout(location = 2) in vec4  a_color;
layout(location = 3) in float a_tex_index;

uniform mat4 u_vp;

out vec2  v_uv;
out vec4  v_color;
out float v_tex_index;

void main() {
    gl_Position = u_vp * vec4(a_pos, 0.0, 1.0);
    v_uv        = a_uv;
    v_color     = a_color;
    v_tex_index = a_tex_index;
}
)GLSL";

inline constexpr const char* kSpriteFrag = R"GLSL(
#ifdef GL_ES
precision mediump float;
#endif

in vec2  v_uv;
in vec4  v_color;
in float v_tex_index;

uniform sampler2D u_textures[8];

out vec4 frag_color;

void main() {
    vec4 tex_color;
    switch (int(v_tex_index)) {
        case 0:  tex_color = texture(u_textures[0], v_uv); break;
        case 1:  tex_color = texture(u_textures[1], v_uv); break;
        case 2:  tex_color = texture(u_textures[2], v_uv); break;
        case 3:  tex_color = texture(u_textures[3], v_uv); break;
        case 4:  tex_color = texture(u_textures[4], v_uv); break;
        case 5:  tex_color = texture(u_textures[5], v_uv); break;
        case 6:  tex_color = texture(u_textures[6], v_uv); break;
        case 7:  tex_color = texture(u_textures[7], v_uv); break;
        default: tex_color = vec4(1.0, 0.0, 1.0, 1.0); break;  // magenta = bad index
    }
    frag_color = tex_color * v_color;
}
)GLSL";

// Platform-specific #version header, prepended before every shader source.
#if defined(GLYPH_PLATFORM_DESKTOP)
inline constexpr const char* kVersionHeader = "#version 330 core\n";
#else
inline constexpr const char* kVersionHeader = "#version 300 es\n";
#endif

} // namespace glyph::shaders
