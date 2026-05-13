// shaders.h — embedded GLSL source strings.
//
// Shaders are written without a #version directive; the platform header is
// prepended at compile time:
//   Desktop GL 3.3:  "#version 330 core\n"
//   GLES 3.0 / Web:  "#version 300 es\n"
//
// #ifdef GL_ES guards precision qualifiers so the same source compiles on
// both profiles without modification.
#pragma once

namespace glyph::shaders {

inline constexpr const char* kSpriteVert = R"GLSL(
layout(location = 0) in vec2 a_pos;
layout(location = 1) in vec2 a_uv;
layout(location = 2) in vec4 a_color;

uniform mat4 u_vp;

out vec2 v_uv;
out vec4 v_color;

void main() {
    gl_Position = u_vp * vec4(a_pos, 0.0, 1.0);
    v_uv    = a_uv;
    v_color = a_color;
}
)GLSL";

inline constexpr const char* kSpriteFrag = R"GLSL(
#ifdef GL_ES
precision mediump float;
#endif

in vec2  v_uv;
in vec4  v_color;

uniform sampler2D u_texture;

out vec4 frag_color;

void main() {
    frag_color = texture(u_texture, v_uv) * v_color;
}
)GLSL";

// Platform-specific #version header, prepended before shader source.
#if defined(GLYPH_PLATFORM_DESKTOP)
inline constexpr const char* kVersionHeader = "#version 330 core\n";
#else
inline constexpr const char* kVersionHeader = "#version 300 es\n";
#endif

} // namespace glyph::shaders
